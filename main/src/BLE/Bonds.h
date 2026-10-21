#ifndef CLOCKSTAR_FIRMWARE_BLE_BONDS_H
#define CLOCKSTAR_FIRMWARE_BLE_BONDS_H

#include <cstring>
#include <vector>
#include <esp_gap_ble_api.h>

namespace BLE {

    // True if bluedroid holds a bond whose stored address equals addr (the pairing-time address).
    inline bool isBonded(const esp_bd_addr_t addr){
        int num = esp_ble_get_bond_device_num();
        if(num <= 0){
            return false;
        }

        std::vector<esp_ble_bond_dev_t> bonded((size_t) num);
        if(esp_ble_get_bond_device_list(&num, bonded.data()) != ESP_OK){
            return false;
        }

        for(int i = 0; i < num; i++){
            if(memcmp(bonded[(size_t) i].bd_addr, addr, ESP_BD_ADDR_LEN) == 0){
                return true;
            }
        }

        return false;
    }

}

#endif //CLOCKSTAR_FIRMWARE_BLE_BONDS_H
