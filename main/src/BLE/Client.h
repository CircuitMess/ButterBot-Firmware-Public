#ifndef CLOCKSTAR_FIRMWARE_BLE_CLIENT_H
#define CLOCKSTAR_FIRMWARE_BLE_CLIENT_H

#include "Util/Queue.h"
#include "Util/PSRAMAllocator.h"
#include <cstdint>
#include <memory>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <esp_bt_defs.h>
#include <esp_gatt_defs.h>
#include <esp_gattc_api.h>

namespace BLE {

class GAP;

class Client {
public:
	class Char;
	class CharInfo;
	class Service;
	class ServiceInfo;
#include "Client/Char.h"
#include "Client/CharInfo.h"
#include "Client/Service.h"
#include "Client/ServiceInfo.h"

	Client(GAP* gap);
	~Client();

	std::shared_ptr<Service> addService(esp_bt_uuid_t uuid);

private:
	static Client* self;
	friend ServiceInfo;
	friend CharInfo;
	friend GAP;

	static constexpr size_t MaxLinks = 2;

	std::unordered_set<std::shared_ptr<Service>> services;
	std::unordered_map<uint16_t, Char*> chars;

	static constexpr int AppID = 0;
	struct InterfaceInfo {
		uint8_t appID = 0xff;
		uint8_t hndl = 0xff;

		operator bool(){ return appID != 0xff && hndl != 0; }
	} iface;

	struct Link {
		bool used = false;
		esp_bd_addr_t addr = {};
		esp_ble_addr_type_t addrType = BLE_ADDR_TYPE_PUBLIC;
		uint16_t connId = 0xffff;
		uint16_t MTU_size = 500;
		bool bonded = false;
		bool securityRequested = false;
		bool paired = false;
		bool opening = false;
		bool opened = false;
		bool discovered = false;
		bool discoveryStarted = false;
		bool mtuDone = false;
	};
	std::array<Link, MaxLinks> links;

	Link* findLinkByConn(uint16_t connId);
	Link* findLinkByAddr(const esp_bd_addr_t addr);
	Link* allocLink();

	struct PendingAuth {
		bool used = false;
		esp_bd_addr_t addr = {};
	};
	std::array<PendingAuth, MaxLinks> pendingAuths;

	void recordPendingAuth(const esp_bd_addr_t addr);
	bool consumePendingAuth(const Link& link);
	void clearPendingAuth(const esp_bd_addr_t addr);

	void maybeStartDiscovery(Link& link);

	void ble_GATTC_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

	GAP* gap;
	void onPairDone(const esp_bd_addr_t addr);
	void onPairFailed(const esp_bd_addr_t addr);

	void requestSecurity(Link& link);

	void onConnect(const esp_ble_gattc_cb_param_t::gattc_connect_evt_param* param);
	void openLink(Link& link);
	void onOpen(const esp_ble_gattc_cb_param_t::gattc_open_evt_param* param);
	void onDiscoveryComplete(const esp_ble_gattc_cb_param_t::gattc_dis_srvc_cmpl_evt_param* param);
	void onMtuResp(const esp_ble_gattc_cb_param_t::gattc_cfg_mtu_evt_param* param);
	void onServiceChanged(const esp_ble_gattc_cb_param_t::gattc_srvc_chg_evt_param* param);

	void searchServices(Link& link);
	void pullServices(Link& link);
	void onSearchResult(const esp_ble_gattc_cb_param_t::gattc_search_res_evt_param* param);
	void onSearchComplete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param* param);

	void onClose(const esp_ble_gattc_cb_param_t::gattc_close_evt_param* param);
	void onDisconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param* param);

	void passToChar(esp_gattc_cb_event_t event, esp_ble_gattc_cb_param_t *param);
	void closeServices(const Link& link);
	void closeLink(Link& link);

};

}


#endif //CLOCKSTAR_FIRMWARE_BLE_CLIENT_H
