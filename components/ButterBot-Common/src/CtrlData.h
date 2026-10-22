#ifndef BUTTERBOT_COMMON_CONTROLLERDATA_H
#define BUTTERBOT_COMMON_CONTROLLERDATA_H

#include <cstdint>

struct Ctrl {
	enum Command {
		EnterRC, ExitRC, Listen, Summon, ShutUp, Poke, Drive, RCSound, Scenario, Settings
	} type;
};

enum class ActivityMode : uint8_t {
	Off, Chill, Active, Hyper
};

enum class FrontSensorMode : uint8_t {
	Off, On
};

enum class BottomSensorMode : uint8_t {
	Off, Reduced, On
};

struct SettingsData {
	FrontSensorMode frontSensor;
	BottomSensorMode bottomSensor;
	ActivityMode activityMode;
};

struct DriveData {
	int8_t joystickX; // [-100, 100]
	int8_t joystickY; // [-100, 100]
};

struct RCData {
	char SSID[16];
	char password[16];
};

#endif //BUTTERBOT_COMMON_CONTROLLERDATA_H