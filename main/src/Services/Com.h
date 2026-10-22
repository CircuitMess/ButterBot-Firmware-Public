#ifndef BUTTERBOT_FIRMWARE_COM_H
#define BUTTERBOT_FIRMWARE_COM_H


#include "BLE/Server.h"
#include <BBData.h>
#include <CtrlData.h>
#include <Scenarios.h>
#include <Entity/AsyncEntity.h>
#include <Event/EventBroadcaster.h>
#include <mutex>

class Com : public AsyncEntity {
	GENERATED_BODY(Com, AsyncEntity, CONSTRUCTOR_PACK(BLE::Server* ))
public:
	Com(BLE::Server* server, bool internalStack = true);
	virtual ~Com() override;

	enum class ConnStatus { Connected, Disconnected };

	DECLARE_EVENT(OnConnStatus, Com, ConnStatus);
	OnConnStatus OnConnStatus{ this };

	ConnStatus getStatus() const;

	DECLARE_EVENT(OnCommand, Com, Ctrl::Command);
	OnCommand OnCommand{ this };

	DECLARE_EVENT(OnDrivedata, Com, DriveData);
	OnDrivedata OnDriveData{ this };

	DECLARE_EVENT(OnRCDataEvent, Com, RCData);
	OnRCDataEvent OnRCData{ this };

	DECLARE_EVENT(OnScenarioEvent, Com, BB::Action::Scenario, ScenarioData);
	OnScenarioEvent OnScenario{ this };

	DECLARE_EVENT(OnSettingsEvent, Com, SettingsData);
	OnSettingsEvent OnSettings{ this };

	void sendData(BB::State state, BB::Action::Idle action);
	void sendData(BB::State state, BB::Action::Scenario action);
	void sendData(BB::State state, BB::Action::RC action);
	void sendData(BB::State state, BB::Action::Idle action, size_t size, const uint8_t* data);
	void sendData(BB::State state, BB::Action::Scenario action, size_t size, const uint8_t* data);
	void sendData(BB::State state, BB::Action::RC action, size_t size, const uint8_t* data);

	template<typename T>
	void sendData(BB::State state, BB::Action::Listen none, const T& data){
		BB::Action _action = {
			.listen = none
		};
		sendData(state, _action, data);
	}

	template<typename T>
	void sendData(BB::State state, BB::Action::Idle action, const T& data){
		BB::Action _action = {
			.idle = action
		};
		sendData(state, _action, data);
	}

	template<typename T>
	void sendData(BB::State state, BB::Action::Scenario action, const T& data){
		BB::Action _action = {
			.scenario = action
		};
		sendData(state, _action, data);
	}

	template<typename T>
	void sendData(BB::State state, BB::Action::RC action, const T& data){
		BB::Action _action = {
			.remoteControl = action
		};
		sendData(state, _action, data);
	}

protected:
	void tick(float deltaTime) noexcept override;

private:
	// Service UUID
	static constexpr esp_bt_uuid_t ServiceUID = {
		.len = ESP_UUID_LEN_128,
		.uuid = { .uuid128 = { 0x19, 0x08, 0x79, 0xbc, 0x80, 0xbd, 0x40, 0x44, 0x91, 0x98, 0x51, 0x82, 0xe7, 0xca, 0x85, 0x36 } }
	};

	// Server-side RX and TX
	static constexpr esp_bt_uuid_t RxCharUID = {
		.len = ESP_UUID_LEN_128,
		.uuid = { .uuid128 = { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E } }
	};
	static constexpr esp_bt_uuid_t TxCharUID = {
		.len = ESP_UUID_LEN_128,
		.uuid = { .uuid128 = { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E } }
	};

	std::shared_ptr<BLE::Server::Service> service;
	std::shared_ptr<BLE::Server::Char> txChar;
	std::shared_ptr<BLE::Server::Char> rxChar;

	BLE::Server::Service::SubHandle disconnectSub = 0;

	ConnStatus status = ConnStatus::Disconnected;

	static constexpr size_t BufSize = 256;
	PSRAMByteBuffer txBuf;
	std::mutex txMutex; // sendData() builds the packet in txBuf; it is called from more than one thread

	void sendData(BB::State state, BB::Action action);
	void sendData(BB::State state, BB::Action action, size_t size, const uint8_t* data);

	template<typename T>
	void sendData(BB::State state, BB::Action action, const T& data){
		static_assert(std::is_base_of<BBData, T>::value, "T must inherit from BBData");
		if constexpr(ComSerializable<T>){
			const std::vector<uint8_t> bytes = data.serialize();
			sendData(state, action, bytes.size(), std::move(bytes.data()));
		} else{
			static_assert(std::is_trivially_copyable_v<T>, "Non-serializable BBData must be trivially copyable to be sent by value");
			sendData(state, action, sizeof(T), reinterpret_cast<const uint8_t*>(&data));
		}
	}

};


#endif //BUTTERBOTCTRL_FIRMWARE_COM_H
