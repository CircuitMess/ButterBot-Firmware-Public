#ifdef CLOCKSTAR_FIRMWARE_BLE_SERVER_H


class Service {
public:
	std::shared_ptr<BLE::Server::Char> addChar(esp_bt_uuid_t uuid, esp_gatt_char_prop_t props = 0);

	// Service-scoped disconnect callback. Fires only when the peer that claimed this service
	// (via a CCCD enable on one of its chars) disconnects — apps that own a service can
	// subscribe here instead of Server::addOnDisconnectCb to avoid spurious teardown when
	// the *other* peer (on a different service) drops.
	using DisconnectCB = std::function<void(const esp_bd_addr_t)>;
	using SubHandle = uint32_t;
	SubHandle addOnDisconnectCb(DisconnectCB cb);
	void removeOnDisconnectCb(SubHandle handle);

	void setPeerNeverPairs();

private:
	friend BLE::Server;
	friend BLE::Server::Char;
	friend BLE::Server::CharInfo;
	Service(esp_gatt_srvc_id_t id);

	BLE::Server* server = nullptr;
	bool peerNeverPairs = false;

	esp_gatt_srvc_id_t id;
	std::vector<std::shared_ptr<BLE::Server::Char>> chars;

	uint16_t hndl = 0xffff;
	void establish(uint16_t hndl);
	std::shared_ptr<Server::Char> charCreated(esp_gatt_status_t status, esp_bt_uuid_t uid, std::unique_ptr<BLE::Server::CharInfo> charInfo);

	// Track which peer claimed this service via a CCCD enable. On disconnect,
	// only services claimed by the disconnecting peer drop per-connection state —
	// chars in services bound to a different still-connected peer are left alone.
	esp_bd_addr_t subscribedPeer = {};
	bool peerClaimed = false;
	// Writes (onPeerSubscribed / onPeerUnsubscribed / onDisconnect) happen on the
	// BLE/BTC task; reads (CharInfo::sendNotif) come from app threads.
	mutable std::mutex peerMut;
	void onPeerSubscribed(const esp_bd_addr_t peer);
	// Called from Char::onWrite when a CCCD disable arrives. Clears `peerClaimed`
	// only if no other char in the service still has notify/indicate enabled.
	void onPeerUnsubscribed(const esp_bd_addr_t peer);
	void onDisconnect(const esp_bd_addr_t peer);

	// Snapshot helper used by CharInfo::sendNotif — returns false (and leaves `out` untouched)
	// if no peer has claimed this service.
	bool getSubscribedPeer(esp_bd_addr_t out) const;

	// Callbacks are added/removed from app threads (Com / Android construction & destruction)
	// and iterated on the BTC task in onDisconnect. Guard with a mutex; iterate over a
	// snapshot taken under the lock so user callbacks don't run with the lock held.
	std::unordered_map<SubHandle, DisconnectCB> onDisconnectCBs;
	SubHandle nextSubHandle = 1;
	mutable std::mutex cbMut;

};


#endif //CLOCKSTAR_FIRMWARE_BLE_SERVER_H
