#include <BLE/Server.h>
#include <algorithm>
#include <cstring>
#include <Log/Log.h>

DEFINE_LOG(BLE_Server_Service)

BLE::Server::Service::Service(esp_gatt_srvc_id_t id) : id(id){

}

std::shared_ptr<BLE::Server::Char> BLE::Server::Service::addChar(esp_bt_uuid_t uuid, esp_gatt_char_prop_t props){
	std::shared_ptr<Char> chr(new Char(uuid, props));
	chr->parent = this;
	chars.push_back(chr);
	return chr;
}

void BLE::Server::Service::setPeerNeverPairs(){
	peerNeverPairs = true;
}

void BLE::Server::Service::onPeerSubscribed(const esp_bd_addr_t peer){
	{
		std::lock_guard lock(peerMut);
		if(peerClaimed && memcmp(subscribedPeer, peer, 6) != 0){
			CMF_LOG(BLE_Server_Service, LogLevel::Warning, "second peer attempted to claim service already claimed by another peer; ignoring");
			return;
		}
		memcpy(subscribedPeer, peer, 6);
		peerClaimed = true;
	}

	if(server != nullptr && peerNeverPairs){
		server->onPeerClaimedUnsecured(peer);
	}
}

void BLE::Server::Service::onPeerUnsubscribed(const esp_bd_addr_t peer){
	std::lock_guard lock(peerMut);
	if(!peerClaimed) return;
	if(memcmp(subscribedPeer, peer, 6) != 0) return;

	// Released only when no char in the service is still subscribed; otherwise the
	// peer is unsubscribing from just one of its chars and still owns the service.
	for(const auto& chr : chars){
		if(chr->notifyEn || chr->indicateEn) return;
	}
	peerClaimed = false;
}

void BLE::Server::Service::onDisconnect(const esp_bd_addr_t peer){
	{
		std::lock_guard lock(peerMut);
		if(!peerClaimed) return;
		if(memcmp(subscribedPeer, peer, 6) != 0) return;
		peerClaimed = false;
	}

	for(auto& chr : chars){
		chr->onDisconnect();
	}

	std::vector<DisconnectCB> cbs;
	{
		std::lock_guard lock(cbMut);
		cbs.reserve(onDisconnectCBs.size());
		for(const auto& [_, cb] : onDisconnectCBs){
			if(cb) cbs.push_back(cb);
		}
	}
	for(const auto& cb : cbs){
		cb(peer);
	}
}

bool BLE::Server::Service::getSubscribedPeer(esp_bd_addr_t out) const{
	std::lock_guard lock(peerMut);
	if(!peerClaimed) return false;
	memcpy(out, subscribedPeer, 6);
	return true;
}

BLE::Server::Service::SubHandle BLE::Server::Service::addOnDisconnectCb(DisconnectCB cb){
	std::lock_guard lock(cbMut);
	const SubHandle handle = nextSubHandle++;
	onDisconnectCBs.emplace(handle, std::move(cb));
	return handle;
}

void BLE::Server::Service::removeOnDisconnectCb(SubHandle handle){
	std::lock_guard lock(cbMut);
	onDisconnectCBs.erase(handle);
}

void BLE::Server::Service::establish(uint16_t hndl){
	CMF_LOG(BLE_Server_Service, LogLevel::Info, "Established");
	this->hndl = hndl;

	esp_ble_gatts_start_service(hndl);

	// Bluedroid attaches a descriptor to the most recently added char in the service
	// ("follow" rule). Char::establish auto-adds a CCCD for chars with NOTIFY/INDICATE,
	// so those chars must be added last for their CCCD to land on them in the GATT
	// table. See the pendingDescrs comment in Server.h for the full rationale.
	std::stable_sort(chars.begin(), chars.end(), [](const std::shared_ptr<Char>& a, const std::shared_ptr<Char>& b){
		constexpr esp_gatt_char_prop_t descrBearing = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_INDICATE;
		return !(a->props & descrBearing) && (b->props & descrBearing);
	});

	for(const auto& chr : chars){
		esp_ble_gatts_add_char(hndl, &chr->uuid, chr->perm, chr->props, nullptr, nullptr);
	}
}

std::shared_ptr<BLE::Server::Char> BLE::Server::Service::charCreated(esp_gatt_status_t status, esp_bt_uuid_t uid, std::unique_ptr<BLE::Server::CharInfo> charInfo){
	auto it = std::find_if(chars.cbegin(), chars.cend(), [uid](const std::shared_ptr<Char>& chr){
		const auto len = std::min(uid.len, chr->uuid.len);
		return memcmp(uid.uuid.uuid128, chr->uuid.uuid.uuid128, len) == 0;
	});
	if(it == chars.end()){
		CMF_LOG(BLE_Server_Service, LogLevel::Warning, "got handle for non-existant service");
		return nullptr;
	}
	auto chr = *it;

	chr->establish(std::move(charInfo));
	return chr;
}
