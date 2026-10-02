#ifdef CLOCKSTAR_FIRMWARE_BLE_CLIENT_H

class ServiceInfo {
public:
	ServiceInfo(BLE::Client* client, uint16_t connId, const esp_bd_addr_t addr, uint16_t startHndl, uint16_t endHndl);

	std::vector<esp_gattc_char_elem_t> getChars() const;
	esp_gattc_char_elem_t getCharByUUID(esp_bt_uuid_t uuid) const;

	std::unique_ptr<BLE::Client::CharInfo> makeCharInfo(uint16_t hndl);
	void regChar(BLE::Client::Char* chr, uint16_t hndl);

	uint16_t getConnId() const;

private:
	BLE::Client* client;

	const uint16_t connId;
	esp_bd_addr_t addr;

	const uint16_t startHndl;
	const uint16_t endHndl;

};

#endif //CLOCKSTAR_FIRMWARE_BLE_CLIENT_H
