#ifndef BUTTERBOT_FIRMWARE_SETTINGS_H
#define BUTTERBOT_FIRMWARE_SETTINGS_H

#include <nvs.h>
#include <BBData.h>
#include <CtrlData.h>

struct SettingsStruct {
	using Scale = TempHumScale;

	bool testVariable = true;
	Scale tempHumScale = Scale::Celsius;
	float ledStrobePeriod = 0.4f;
	float ledBreathePeriod = 2.0f;
	bool gasModuleConfigured = false;
	FrontSensorMode frontSensor = FrontSensorMode::On;
	BottomSensorMode bottomSensor = BottomSensorMode::Reduced;
	ActivityMode activityMode = ActivityMode::Active;
};

class Settings {
public:
	Settings();
	virtual ~Settings();

	SettingsStruct get() const;
	void set(const SettingsStruct& settings);
	void store() const;

private:
	SettingsStruct settingsStruct;

	static constexpr const char* BlobName = "Settings";
	static constexpr const char* NVSNamespace = "Butterbot";
	nvs_handle_t handle{};

	void load();
};


#endif //BUTTERBOT_FIRMWARE_SETTINGS_H