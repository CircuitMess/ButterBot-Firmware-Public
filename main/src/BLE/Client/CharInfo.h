#ifdef CLOCKSTAR_FIRMWARE_BLE_CLIENT_H

class CharInfo {
public:
	CharInfo(const BLE::Client* client, uint16_t connId, const esp_bd_addr_t addr, uint16_t hndl);

	void regForNotify();

	void writeDescr(esp_bt_uuid_t uuid, uint8_t* data, size_t len);

	void write(uint8_t* data, size_t len, bool needResponse);

	void read();

private:
	const BLE::Client* client;

	const uint16_t connId;
	esp_bd_addr_t addr;

	const uint16_t hndl;

};

#endif //CLOCKSTAR_FIRMWARE_BLE_CLIENT_H
