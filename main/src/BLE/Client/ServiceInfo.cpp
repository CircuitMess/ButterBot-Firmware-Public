#include "../Client.h"
#include "Util/UUIDFmt.h"
#include <cstring>
#include <Log/Log.h>

DEFINE_LOG(BLE_Client_ServiceInfo)

BLE::Client::ServiceInfo::ServiceInfo(Client* client, uint16_t connId, const esp_bd_addr_t addr, uint16_t startHndl, uint16_t endHndl) : client(client), connId(connId), startHndl(startHndl), endHndl(endHndl){
	memcpy(this->addr, addr, ESP_BD_ADDR_LEN);
}

std::vector<esp_gattc_char_elem_t> BLE::Client::ServiceInfo::getChars() const{
	uint16_t count;
	esp_gatt_status_t ret = esp_ble_gattc_get_attr_count(client->iface.hndl, connId, ESP_GATT_DB_CHARACTERISTIC, startHndl, endHndl, 0, &count);
	if(ret != ESP_GATT_OK){
		CMF_LOG(BLE_Client_ServiceInfo, LogLevel::Error, "get_attr_count error 0x%x", ret);
		return {};
	}

	std::vector<esp_gattc_char_elem_t> chars(count);

	ret = esp_ble_gattc_get_all_char(client->iface.hndl, connId, startHndl, endHndl, chars.data(), &count, 0);
	if(ret != ESP_GATT_OK){
		CMF_LOG(BLE_Client_ServiceInfo, LogLevel::Error, "get_all_char error 0x%x", ret);
		return {};
	}

	return chars;
}

esp_gattc_char_elem_t BLE::Client::ServiceInfo::getCharByUUID(esp_bt_uuid_t uuid) const{
	esp_gattc_char_elem_t chr;
	uint16_t count = 1;

	const esp_gatt_status_t ret = esp_ble_gattc_get_char_by_uuid(client->iface.hndl, connId, startHndl, endHndl, uuid, &chr, &count);
	if(ret != ESP_GATT_OK){
		CMF_LOG(BLE_Client_ServiceInfo, LogLevel::Error, "get_char_by_uuid error 0x%x for uuid %s", ret, formatUUID128(uuid.uuid.uuid128).c_str());
		return {};
	}

	if(count == 0){
		CMF_LOG(BLE_Client_ServiceInfo, LogLevel::Warning, "getCharByUUID returned 0 results");
		return {};
	}

	return chr;
}

std::unique_ptr<BLE::Client::CharInfo> BLE::Client::ServiceInfo::makeCharInfo(uint16_t hndl){
	return std::make_unique<CharInfo>(client, connId, addr, hndl);
}

void BLE::Client::ServiceInfo::regChar(Char* chr, uint16_t hndl){
	client->chars.insert(std::make_pair(hndl, chr));
}

uint16_t BLE::Client::ServiceInfo::getConnId() const{
	return connId;
}
