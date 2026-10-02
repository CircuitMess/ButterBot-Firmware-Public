#include "Client.h"
#include "GAP.h"
#include "Bonds.h"
#include <cstring>
#include <esp_mac.h>
#include <esp_gap_ble_api.h>
#include <Log/Log.h>

DEFINE_LOG(BLE_Client)

BLE::Client* BLE::Client::self = nullptr;

BLE::Client::Client(GAP* gap) : gap(gap){
	if(self != nullptr){
		CMF_LOG(BLE_Client, LogLevel::Error,"Client already exists");
		return;
	}
	self = this;

	esp_ble_gattc_register_callback([](esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param){
		if(self == nullptr) return;
		self->ble_GATTC_cb(event, gattc_if, param);
	});

	if(iface.appID == 0xff){
		iface.appID = AppID;
		esp_ble_gattc_app_register(AppID);
	}

	// TODO: This is only needed so GAP can notify the GATT Client when pairing is done
	gap->setClient(this);
}

BLE::Client::~Client(){
	self = nullptr;
	gap->setClient(nullptr);
}

std::shared_ptr<BLE::Client::Service> BLE::Client::addService(esp_bt_uuid_t uuid){
	std::shared_ptr<Service> srv(new Service(uuid));
	services.insert(srv);
	return srv;
}

void BLE::Client::ble_GATTC_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param){
	/* 0xff, not specify a certain gatt_if, need to call every profile cb function */
	if(gattc_if == 0xff){
		printf("GATTC CB interface 0xff - event %d\n", event);
	}

	if(event == ESP_GATTC_REG_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_REG_EVT");

		if(param->reg.app_id != iface.appID){
			CMF_LOG(BLE_Client, LogLevel::Error,"CB: App ID missmatch. Received %d, have %d. Status is %d", param->reg.app_id, iface.appID, param->reg.status);
			return;
		}

		if(param->reg.status == ESP_GATT_OK){
			CMF_LOG(BLE_Client, LogLevel::Info,"App registered");
			iface.hndl = gattc_if;
		}else{
			CMF_LOG(BLE_Client, LogLevel::Warning,"Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
			iface = {};
			return;
		}
	}else if(event == ESP_GATTC_CONNECT_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_CONNECT_EVT conn %d addr " MACSTR " type %d role %d", param->connect.conn_id, MAC2STR(param->connect.remote_bda), param->connect.ble_addr_type, param->connect.link_role);
		onConnect(&param->connect);
	}else if(event == ESP_GATTC_OPEN_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_OPEN_EVT");
		onOpen(&param->open);
	}else if(event == ESP_GATTC_DIS_SRVC_CMPL_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_DIS_SRVC_CMPL_EVT");
		onDiscoveryComplete(&param->dis_srvc_cmpl);
	}else if(event == ESP_GATTC_CFG_MTU_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_CFG_MTU_EVT");
		onMtuResp(&param->cfg_mtu);
	}else if(event == ESP_GATTC_SRVC_CHG_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_SRVC_CHG_EVT");
		onServiceChanged(&param->srvc_chg);
	}else if(event == ESP_GATTC_SEARCH_RES_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_SEARCH_RES_EVT");
		onSearchResult(&param->search_res);
	}else if(event == ESP_GATTC_SEARCH_CMPL_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_SEARCH_CMPL_EVT");
		onSearchComplete(&param->search_cmpl);
	}else if(event == ESP_GATTC_CLOSE_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_CLOSE_EVT");
		onClose(&param->close);
	}else if(event == ESP_GATTC_DISCONNECT_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_DISCONNECT_EVT");
		onDisconnect(&param->disconnect);
	}else{
		switch(event){
			case ESP_GATTC_REG_FOR_NOTIFY_EVT:
			case ESP_GATTC_NOTIFY_EVT:
			case ESP_GATTC_WRITE_CHAR_EVT:
			case ESP_GATTC_READ_CHAR_EVT:
				passToChar(event, param);
				break;
			default:
				CMF_LOG(BLE_Client, LogLevel::Verbose,"Unhandled event: %d", event);
				break;
		}
	}
}

BLE::Client::Link* BLE::Client::findLinkByConn(uint16_t connId){
	for(Link& link : links){
		if(link.used && link.connId == connId){
			return &link;
		}
	}

	return nullptr;
}

BLE::Client::Link* BLE::Client::findLinkByAddr(const esp_bd_addr_t addr){
	for(Link& link : links){
		if(link.used && memcmp(link.addr, addr, ESP_BD_ADDR_LEN) == 0){
			return &link;
		}
	}

	return nullptr;
}

BLE::Client::Link* BLE::Client::allocLink(){
	for(Link& link : links){
		if(!link.used){
			link = Link{};
			link.used = true;
			return &link;
		}
	}

	return nullptr;
}

void BLE::Client::requestSecurity(Link& link){
	link.securityRequested = true;

	const esp_err_t err = esp_ble_set_encryption(link.addr, ESP_BLE_SEC_ENCRYPT_MITM);
	if(err != ESP_OK){
		CMF_LOG(BLE_Client, LogLevel::Warning, "esp_ble_set_encryption failed: 0x%x", err);
		link.securityRequested = false;
	}
}

void BLE::Client::onConnect(const esp_ble_gattc_cb_param_t::gattc_connect_evt_param* param){
	if(findLinkByConn(param->conn_id) != nullptr){
		return;
	}

	Link* link = allocLink();
	if(link == nullptr){
		CMF_LOG(BLE_Client, LogLevel::Warning, "connect: capacity exceeded, ignoring conn %d", param->conn_id);
		return;
	}

	memcpy(link->addr, param->remote_bda, ESP_BD_ADDR_LEN);
	link->addrType = param->ble_addr_type;
	link->connId = param->conn_id;
	link->bonded = BLE::isBonded(param->remote_bda);

	if(consumePendingAuth(*link)){
		CMF_LOG(BLE_Client, LogLevel::Info, "link was encrypted before the connect event, resuming");
		link->paired = true;
	}

	openLink(*link);
	maybeStartDiscovery(*link);
}

void BLE::Client::maybeStartDiscovery(Link& link){
	if(!link.opened || !link.discovered || link.discoveryStarted){
		return;
	}

	link.discoveryStarted = true;

	if(link.mtuDone){
		searchServices(link);
		return;
	}

	const esp_err_t err = esp_ble_gattc_send_mtu_req(iface.hndl, link.connId);
	if(err != ESP_OK){
		CMF_LOG(BLE_Client, LogLevel::Warning, "esp_ble_gattc_send_mtu_req failed: 0x%x", err);
		link.mtuDone = true;
		searchServices(link);
	}
}

void BLE::Client::recordPendingAuth(const esp_bd_addr_t addr){
	for(PendingAuth& pending : pendingAuths){
		if(pending.used && memcmp(pending.addr, addr, ESP_BD_ADDR_LEN) == 0){
			return;
		}
	}

	for(PendingAuth& pending : pendingAuths){
		if(!pending.used){
			pending.used = true;
			memcpy(pending.addr, addr, ESP_BD_ADDR_LEN);
			return;
		}
	}

	CMF_LOG(BLE_Client, LogLevel::Warning, "pending auth storage full, dropping");
}

bool BLE::Client::consumePendingAuth(const Link& link){
	for(PendingAuth& pending : pendingAuths){
		if(pending.used && memcmp(pending.addr, link.addr, ESP_BD_ADDR_LEN) == 0){
			pending = PendingAuth{};
			return true;
		}
	}

	if(!link.bonded){
		return false;
	}

	PendingAuth* only = nullptr;
	size_t count = 0;
	for(PendingAuth& pending : pendingAuths){
		if(pending.used){
			only = &pending;
			count++;
		}
	}

	if(count != 1){
		return false;
	}

	*only = PendingAuth{};
	return true;
}

void BLE::Client::clearPendingAuth(const esp_bd_addr_t addr){
	for(PendingAuth& pending : pendingAuths){
		if(pending.used && memcmp(pending.addr, addr, ESP_BD_ADDR_LEN) == 0){
			pending = PendingAuth{};
		}
	}
}

void BLE::Client::onPairDone(const esp_bd_addr_t addr){
	Link* link = findLinkByAddr(addr);
	if(link == nullptr){
		Link* unpaired = nullptr;
		size_t unpairedCount = 0;
		for(Link& candidate : links){
			if(candidate.used && !candidate.paired){
				unpaired = &candidate;
				unpairedCount++;
			}
		}

		if(unpairedCount != 1){
			CMF_LOG(BLE_Client, LogLevel::Info, "pairing completed before the connect event, remembering it");
			recordPendingAuth(addr);
			return;
		}

		CMF_LOG(BLE_Client, LogLevel::Warning, "pairing completed under an address other than the connect-time one, taking it for the only unpaired link");
		link = unpaired;
	}

	link->paired = true;
	link->bonded = true;
	link->securityRequested = false;

	pullServices(*link);
}

void BLE::Client::onPairFailed(const esp_bd_addr_t addr){
	Link* link = findLinkByAddr(addr);
	if(link != nullptr){
		link->securityRequested = false;
		link->bonded = false;
	}else{
		for(Link& candidate : links){
			candidate.securityRequested = false;
		}
	}
}

void BLE::Client::openLink(Link& link){
	link.opening = true;

	const esp_err_t err = esp_ble_gattc_open(iface.hndl, link.addr, link.addrType, true);
	if(err != ESP_OK){
		CMF_LOG(BLE_Client, LogLevel::Error, "esp_ble_gattc_open failed: 0x%x", err);
		link.opening = false;
	}
}

void BLE::Client::onOpen(const esp_ble_gattc_cb_param_t::gattc_open_evt_param* param){
	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		link = findLinkByAddr(param->remote_bda);
	}

	if(link == nullptr){
		CMF_LOG(BLE_Client, LogLevel::Warning, "open event for an untracked link, conn %d", param->conn_id);
		return;
	}

	link->opening = false;

	if(param->status != ESP_GATT_OK){
		CMF_LOG(BLE_Client, LogLevel::Error,"open failed, error status = 0x%x", param->status);
		return;
	}

	link->connId = param->conn_id;
	link->opened = true;
	maybeStartDiscovery(*link);
}

void BLE::Client::onDiscoveryComplete(const esp_ble_gattc_cb_param_t::gattc_dis_srvc_cmpl_evt_param* param){
	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		return;
	}

	if(param->status != ESP_GATT_OK){
		CMF_LOG(BLE_Client, LogLevel::Error, "service discovery failed, error status = 0x%x", param->status);
		return;
	}

	link->discovered = true;
	maybeStartDiscovery(*link);
}

void BLE::Client::onMtuResp(const esp_ble_gattc_cb_param_t::gattc_cfg_mtu_evt_param* param){
	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		return;
	}

	if(param->status != ESP_GATT_OK){
		CMF_LOG(BLE_Client, LogLevel::Error,"config mtu failed, error status = 0x%x", param->status);
	}else{
		link->MTU_size = param->mtu;
	}

	if(link->mtuDone){
		return;
	}

	link->mtuDone = true;

	if(link->discoveryStarted){
		searchServices(*link);
	}
}

void BLE::Client::onServiceChanged(const esp_ble_gattc_cb_param_t::gattc_srvc_chg_evt_param* param){
	Link* link = findLinkByAddr(param->remote_bda);
	if(link == nullptr){
		return;
	}

	CMF_LOG(BLE_Client, LogLevel::Info, "remote database changed, dropping services until rediscovery completes");
	closeServices(*link);

	link->discovered = false;
	link->discoveryStarted = false;
}

void BLE::Client::searchServices(Link& link){
	for(const std::shared_ptr<Service>& service : services){
		esp_ble_gattc_search_service(iface.hndl, link.connId, &service->uuid);
	}
}

void BLE::Client::onSearchResult(const esp_ble_gattc_cb_param_t::gattc_search_res_evt_param* param){
	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		return;
	}

	for(const std::shared_ptr<Service>& service : services){
		if(param->srvc_id.uuid.len != service->uuid.len || memcmp(param->srvc_id.uuid.uuid.uuid128, service->uuid.uuid.uuid128, service->uuid.len) != 0){
			continue;
		}

		if(service->established()){
			continue;
		}

		service->establish(std::make_unique<ServiceInfo>(this, link->connId, link->addr, param->start_handle, param->end_handle));
	}
}

void BLE::Client::onSearchComplete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param* param){
	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		return;
	}

	if(param->status != ESP_GATT_OK){
		CMF_LOG(BLE_Client, LogLevel::Error,"search service failed, error status = %x", param->status);
		return;
	}

	pullServices(*link);
}

void BLE::Client::pullServices(Link& link){
	bool waiting = false;

	for(const std::shared_ptr<Service>& svc : services){
		if(!svc->established() || svc->populated() || svc->svc->getConnId() != link.connId){
			continue;
		}

		if(!link.paired){
			waiting = true;
			continue;
		}

		svc->pull();
	}

	if(!waiting || link.paired || link.securityRequested){
		return;
	}

	CMF_LOG(BLE_Client, LogLevel::Info, "registered service found on an unencrypted link, requesting security");
	requestSecurity(link);
}

void BLE::Client::onClose(const esp_ble_gattc_cb_param_t::gattc_close_evt_param* param){
	if(param->status != ESP_GATT_OK){
		CMF_LOG(BLE_Client, LogLevel::Error,"close failed, error status = %x", param->status);
		return;
	}

	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		return;
	}

	closeLink(*link);
}

void BLE::Client::onDisconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param* param){
	CMF_LOG(BLE_Client, LogLevel::Info,"Disconnected. Reason: 0x%x, conn %d addr " MACSTR, param->reason, param->conn_id, MAC2STR(param->remote_bda));

	clearPendingAuth(param->remote_bda);

	Link* link = findLinkByConn(param->conn_id);
	if(link == nullptr){
		link = findLinkByAddr(param->remote_bda);
	}

	if(link == nullptr){
		return;
	}

	closeLink(*link);
}

void BLE::Client::passToChar(esp_gattc_cb_event_t event, esp_ble_gattc_cb_param_t* param){
#define check(x) do { if(x == chars.end()){ CMF_LOG(BLE_Client, LogLevel::Warning,"Received event %d directed to non-registered characteristic", event); return; } } while(0)

	if(event == ESP_GATTC_REG_FOR_NOTIFY_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_REG_FOR_NOTIFY_EVT");

		auto chr = chars.find(param->reg_for_notify.handle);
		check(chr);

		chr->second->onRegNotify(&param->reg_for_notify);
	}else if(event == ESP_GATTC_NOTIFY_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_NOTIFY_EVT");

		auto chr = chars.find(param->notify.handle);
		check(chr);

		chr->second->onNotify(&param->notify);
	}else if(event == ESP_GATTC_READ_CHAR_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_READ_CHAR_EVT");

		auto chr = chars.find(param->read.handle);
		check(chr);

		// TODO: lol what is this, turn this into actual read evt handling
		esp_ble_gattc_cb_param_t::gattc_notify_evt_param notifParam = {
				.value_len = param->read.value_len,
				.value = param->read.value,
				.is_notify = true
		};
		chr->second->onNotify(&notifParam);
	}else if(event == ESP_GATTC_WRITE_CHAR_EVT){
		CMF_LOG(BLE_Client, LogLevel::Info,"ESP_GATTC_WRITE_CHAR_EVT");

		auto chr = chars.find(param->write.handle);
		check(chr);

		chr->second->onWriteResp(event, &param->write);

	}else{
		CMF_LOG(BLE_Client, LogLevel::Warning,"Unhandled characteristic event: 0x%x", event);
	}
}

void BLE::Client::closeServices(const Link& link){
	for(const std::shared_ptr<Service>& svc : services){
		if(svc == nullptr || !svc->established() || svc->svc->getConnId() != link.connId){
			continue;
		}

		for(const std::shared_ptr<Char>& chr : svc->chars){
			std::unordered_map<uint16_t, Char*>::iterator it = chars.begin();
			while(it != chars.end()){
				if(it->second == chr.get()){
					it = chars.erase(it);
				}else{
					++it;
				}
			}
		}

		svc->close();
	}
}

void BLE::Client::closeLink(Link& link){
	closeServices(link);
	clearPendingAuth(link.addr);
	link = Link{};
}
