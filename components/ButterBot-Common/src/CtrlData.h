#ifndef BUTTERBOT_COMMON_CONTROLLERDATA_H
#define BUTTERBOT_COMMON_CONTROLLERDATA_H

#include <cstdint>

struct Ctrl {
	enum Command {
		EnterRC, ExitRC, Listen, Summon, ShutUp, Poke, Drive, RCSound, Scenario,
		// Proximity sensor filter (appended to keep existing values stable)
		SensorsAllOn, SensorsFrontOff, SensorsFloorOff, SensorsAllOff
	} type;
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