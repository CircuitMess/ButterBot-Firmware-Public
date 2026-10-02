#include "ConMan.h"
#include "Bonds.h"
#include <cstring>
#include <esp_mac.h>
#include <Log/Log.h>

DEFINE_LOG(BLE_ConMan)

ConManager ConMan;

void ConManager::confDone(bool success){
	conConf.confDone(success);
}

void ConManager::start(){
	startAdv();
}

void ConManager::connect(const esp_bd_addr_t addr){
	// Bluedroid implicitly stops advertising on a connection and does not always fire
	// ADV_STOP_COMPLETE_EVT for it. Sync the host-side hint here so the startAdv() /
	// stopAdv() short-circuits below match the real radio state.
	advertising = false;

	Peer* p = findPeer(addr);
	if(!p){
		for(auto& slot : peers){
			if(!slot.used){
				slot.used = true;
				memcpy(slot.addr, addr, 6);
				slot.confDeferred = false;
				slot.settling = false;
				p = &slot;
				break;
			}
		}
	}
	if(!p){
		CMF_LOG(BLE_ConMan, LogLevel::Warning, "connect: capacity exceeded; bluedroid accepted more than MaxPeers");
		return;
	}

	if(consumePreSettled(addr)){
		CMF_LOG(BLE_ConMan, LogLevel::Info, "connect: " MACSTR " was secured before the connect event", MAC2STR(addr));
		setCon(p->addr);
	}else{
		p->settling = true;

		if(BLE::isBonded(addr)){
			CMF_LOG(BLE_ConMan, LogLevel::Info, "connect: " MACSTR " is bonded, deferring conn-param update until it re-encrypts", MAC2STR(addr));
			p->confDeferred = true;
		}else{
			setCon(p->addr);
		}
	}

	if(peerCount() >= MaxPeers){
		stopAdv();
	}else{
		startAdv();
	}
}

void ConManager::onSecured(const esp_bd_addr_t addr){
	Peer* p = findSettlingPeer(addr, "secured");
	if(!p){
		for(auto& slot : preSettled){
			if(slot.used) continue;
			slot.used = true;
			memcpy(slot.addr, addr, 6);
			CMF_LOG(BLE_ConMan, LogLevel::Info, "secured: " MACSTR " not connected yet, remembering", MAC2STR(addr));
			return;
		}
		CMF_LOG(BLE_ConMan, LogLevel::Warning, "secured: " MACSTR " not connected and no room to remember it", MAC2STR(addr));
		return;
	}

	if(p->confDeferred){
		p->confDeferred = false;
		CMF_LOG(BLE_ConMan, LogLevel::Info, "secured: sending the deferred conn-param update for " MACSTR, MAC2STR(p->addr));
		setCon(p->addr);
	}

	settle(*p, "secured");
}

void ConManager::onSecurityFailed(const esp_bd_addr_t addr){
	Peer* p = findSettlingPeer(addr, "security failed");
	if(!p) return;

	settle(*p, "security failed");
}

void ConManager::onPeerWillNotPair(const esp_bd_addr_t addr){
	Peer* p = findSettlingPeer(addr, "will not pair");
	if(!p) return;

	settle(*p, "claimed a service that never pairs");
}

void ConManager::settle(Peer& p, const char* why){
	if(!p.settling) return;
	p.settling = false;

	CMF_LOG(BLE_ConMan, LogLevel::Info, "peer " MACSTR " settled (%s)%s", MAC2STR(p.addr), why, anySettling() ? ", another peer still settling" : "");

	if(!anySettling() && wantAdv){
		startAdv();
	}
}


bool ConManager::consumePreSettled(const esp_bd_addr_t addr){
	for(auto& slot : preSettled){
		if(slot.used && memcmp(slot.addr, addr, 6) == 0){
			slot.used = false;
			return true;
		}
	}

	PreSettled* only = nullptr;
	size_t count = 0;
	for(auto& slot : preSettled){
		if(slot.used){
			only = &slot;
			count++;
		}
	}
	if(count == 1 && peerCount() == 1){
		CMF_LOG(BLE_ConMan, LogLevel::Warning, "connect: " MACSTR " taken as the peer secured earlier under " MACSTR, MAC2STR(addr), MAC2STR(only->addr));
		only->used = false;
		return true;
	}

	return false;
}

bool ConManager::anySettling() const{
	for(const auto& p : peers){
		if(p.used && p.settling) return true;
	}
	return false;
}

ConManager::Peer* ConManager::findSettlingPeer(const esp_bd_addr_t addr, const char* what){
	if(Peer* p = findPeer(addr)) return p;

	Peer* only = nullptr;
	size_t settling = 0;
	for(auto& slot : peers){
		if(slot.used && slot.settling){
			only = &slot;
			settling++;
		}
	}
	if(settling != 1) return nullptr;

	CMF_LOG(BLE_ConMan, LogLevel::Warning, "%s: " MACSTR " not tracked, taking it for the only settling peer " MACSTR, what, MAC2STR(addr), MAC2STR(only->addr));
	return only;
}

void ConManager::disconnect(const esp_bd_addr_t addr){
	if(Peer* p = findPeer(addr)){
		p->used = false;
		p->confDeferred = false;
		p->settling = false;
		memset(p->addr, 0, 6);
	}

	for(auto& slot : preSettled){
		if(slot.used && memcmp(slot.addr, addr, 6) == 0){
			slot.used = false;
		}
	}

	// Only wipe in-flight conn-param state when the last peer has gone; otherwise the
	// remaining peer's pending conf would be cleared along with the disconnecting one's.
	if(peerCount() == 0){
		conConf.reset();
	}

	// Decide based on current peer count rather than assuming the disconnect freed a slot:
	// if `addr` wasn't recorded (e.g. a connect that hit the capacity-exceeded path) the
	// slots are still full and we must not re-start advertising.
	if(peerCount() < MaxPeers){
		startAdv();
	}
}

void ConManager::goLowPow(){
	lowPow = true;
	for(const auto& p : peers){
		if(p.used && !p.confDeferred) setCon(p.addr);
	}
	if(peerCount() < MaxPeers){
		startAdv();
	}
}

void ConManager::goHiPow(){
	lowPow = false;
	for(const auto& p : peers){
		if(p.used && !p.confDeferred) setCon(p.addr);
	}
	if(peerCount() < MaxPeers){
		startAdv();
	}
}

void ConManager::startAdv(){
	wantAdv = true;
	if(advertising) return;
	if(anySettling()){
		CMF_LOG(BLE_ConMan, LogLevel::Info, "holding advertising until the connected peer's security settles");
		return;
	}
	auto err = esp_ble_gap_start_advertising((esp_ble_adv_params_t*) (lowPow ? &AdvLowPow : &AdvHiPow));
	if(err != ESP_OK){
		CMF_LOG(BLE_ConMan, LogLevel::Warning, "esp_ble_gap_start_advertising failed: 0x%x", err);
	}
}

void ConManager::stopAdv(){
	wantAdv = false;
	if(!advertising) return;
	auto err = esp_ble_gap_stop_advertising();
	if(err != ESP_OK){
		CMF_LOG(BLE_ConMan, LogLevel::Warning, "esp_ble_gap_stop_advertising failed: 0x%x", err);
	}
}

void ConManager::onAdvStartComplete(bool success){
	if(!success){
		advertising = false;
		// Honest divergence: GAP rejected our start. Don't auto-retry here — the caller's next
		// state change (next connect/disconnect/goLowPow/goHiPow) will issue another start.
		return;
	}
	advertising = true;

	// If our intent has flipped to "stop" between issuing the start and the event arriving,
	// pull the trigger now to converge.
	if(!wantAdv){
		auto err = esp_ble_gap_stop_advertising();
		if(err != ESP_OK){
			CMF_LOG(BLE_ConMan, LogLevel::Warning, "esp_ble_gap_stop_advertising failed: 0x%x", err);
		}
	}
}

void ConManager::onAdvStopComplete(bool success){
	if(success){
		advertising = false;
	}

	// If our intent flipped to "start" between issuing the stop and the event arriving, resync.
	if(wantAdv && !advertising){
		startAdv();
	}
}

void ConManager::setCon(const esp_bd_addr_t addr){
	/* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
	esp_ble_conn_update_params_t params = {};
	memcpy(&params, lowPow ? &ConLowPow : &ConHiPow, sizeof(esp_ble_conn_update_params_t));
	memcpy(params.bda, addr, 6);
	conConf.conf(params);
}

size_t ConManager::peerCount() const{
	size_t n = 0;
	for(const auto& p : peers){
		if(p.used) ++n;
	}
	return n;
}

ConManager::Peer* ConManager::findPeer(const esp_bd_addr_t addr){
	for(auto& p : peers){
		if(p.used && memcmp(p.addr, addr, 6) == 0) return &p;
	}
	return nullptr;
}
