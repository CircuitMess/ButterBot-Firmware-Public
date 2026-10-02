#ifndef CLOCKSTAR_FIRMWARE_CONMAN_H
#define CLOCKSTAR_FIRMWARE_CONMAN_H

#include "GAP.h"
#include "ConConf.h"
#include <esp_gap_ble_api.h>
#include <array>
#include <cstring>

class ConManager {
public:

	// Capacity for simultaneous links: phone (NUS) + controller (Com).
	static constexpr size_t MaxPeers = 2;

	void connect(const esp_bd_addr_t addr);
	void disconnect(const esp_bd_addr_t addr);

	void onSecured(const esp_bd_addr_t addr);
	void onSecurityFailed(const esp_bd_addr_t addr);
	void onPeerWillNotPair(const esp_bd_addr_t addr);

	// Called once the GAP config (adv/scan-response data) is ready, to kick off advertising.
	void start();

	void goLowPow();
	void goHiPow();

private:
	friend BLE::GAP;
	void confDone(bool success);
	ConConf conConf;

	struct Peer {
		bool used = false;
		esp_bd_addr_t addr;
		bool confDeferred = false;
		bool settling = false;
	};
	std::array<Peer, MaxPeers> peers;

	struct PreSettled {
		bool used = false;
		esp_bd_addr_t addr;
	};
	std::array<PreSettled, MaxPeers> preSettled;

	size_t peerCount() const;
	Peer* findPeer(const esp_bd_addr_t addr);
	Peer* findSettlingPeer(const esp_bd_addr_t addr, const char* what);
	bool anySettling() const;
	void settle(Peer& p, const char* why);
	bool consumePreSettled(const esp_bd_addr_t addr);

	bool advertising = false;
	bool wantAdv = false;

	bool lowPow = false;

	void startAdv();
	void stopAdv();
	void setCon(const esp_bd_addr_t addr);

	// Fired by GAP from ADV_START/ADV_STOP complete events on the BLE/BTC task.
	void onAdvStartComplete(bool success);
	void onAdvStopComplete(bool success);

	static constexpr esp_ble_adv_params_t AdvLowPow = {
			.adv_int_min        = 2056, // 1285ms = 2056 * 0.625ms
			.adv_int_max        = 2056, // Apple: Interval should be 20ms. After 30 seconds of no connection, feel free to switch to 1285ms
			.adv_type           = ADV_TYPE_IND,
			.own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
			.channel_map        = ADV_CHNL_ALL,
			.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
	};

	static constexpr esp_ble_adv_params_t AdvHiPow = {
			.adv_int_min        = 32, // 20ms = 32 * 0.625ms
			.adv_int_max        = 32,
			.adv_type           = ADV_TYPE_IND,
			.own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
			.channel_map        = ADV_CHNL_ALL,
			.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
	};

	static constexpr esp_ble_conn_update_params_t ConLowPow = {
			.min_int = 720, // min_int = 720*1.25ms = 900ms
			.max_int = 960, // max_int = 960*1.25ms = 1125ms
			.latency = 0,
			.timeout = 600 // timeout = 500*10ms = 5000ms
	};

	static constexpr esp_ble_conn_update_params_t ConHiPow = {
			.min_int = 24, // min_int = 24*1.25ms = 30ms
			.max_int = 40, // max_int = 40*1.25ms = 50ms
			.latency = 0,
			.timeout = 400 // timeout = 500*10ms = 5000ms
	};

};

extern ConManager ConMan;

#endif //CLOCKSTAR_FIRMWARE_CONMAN_H
