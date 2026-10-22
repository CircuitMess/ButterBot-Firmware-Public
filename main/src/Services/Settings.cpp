#include "Settings.h"
#include <nvs_flash.h>

DEFINE_LOG(Settings)

Settings::Settings(){
	ESP_ERROR_CHECK(nvs_open(NVSNamespace, NVS_READWRITE, &handle));
	load();
}

Settings::~Settings(){
	nvs_close(handle);
}

SettingsStruct Settings::get() const{
	return settingsStruct;
}

void Settings::set(const SettingsStruct& settings){
	settingsStruct = settings;
}

void Settings::store() const{
	esp_err_t err = nvs_set_blob(handle, BlobName, &settingsStruct, sizeof(SettingsStruct));
	if(err != ESP_OK){
		CMF_LOG(Settings, LogLevel::Error, "Error storing settings: %s", esp_err_to_name(err));
		return;
	}
	nvs_commit(handle);
}

void Settings::load(){
	size_t len = sizeof(SettingsStruct);
	esp_err_t err = nvs_get_blob(handle, BlobName, &settingsStruct, &len);
	if(err != ESP_OK){
		CMF_LOG(Settings, LogLevel::Warning, "No stored settings found, using defaults: %s", esp_err_to_name(err));
		settingsStruct = SettingsStruct();
	}

	switch(settingsStruct.frontSensor){
		case FrontSensorMode::Off:
		case FrontSensorMode::On:
			break;
		default:
			settingsStruct.frontSensor = SettingsStruct().frontSensor;
			break;
	}

	switch(settingsStruct.bottomSensor){
		case BottomSensorMode::Off:
		case BottomSensorMode::Reduced:
		case BottomSensorMode::On:
			break;
		default:
			settingsStruct.bottomSensor = SettingsStruct().bottomSensor;
			break;
	}

	switch(settingsStruct.activityMode){
		case ActivityMode::Off:
		case ActivityMode::Chill:
		case ActivityMode::Active:
		case ActivityMode::Hyper:
			break;
		default:
			settingsStruct.activityMode = SettingsStruct().activityMode;
			break;
	}
}