#ifndef BUTTERBOT_FIRMWARE_JIGHWTEST_H
#define BUTTERBOT_FIRMWARE_JIGHWTEST_H

#include <vector>
#include "Battery/Battery.h"
#include <esp_spiffs.h>
#include <Devices/BM8563.h>
#include <Devices/TCA9555.h>
#include "Util/HardwareConfiguration.h"
#include "Periphery/I2CMaster.h"

struct Test {
	bool (* test)();
	const char* name;
	void (* onFail)();
};

class JigHWTest : public Object{
	GENERATED_BODY(JigHWTest, Object, void)

public:
	JigHWTest();
	static bool checkJig();
	void start();

private:
	inline static StrongObjectPtr<I2CMaster> i2cMain = nullptr;
	// Must be careful when using tca and rtc, they are not initialized before their check test passes
	inline static StrongObjectPtr<TCA9555> tca = nullptr;
	inline static StrongObjectPtr<BM8563> rtc = nullptr;
	inline static JigHWTest* test = nullptr;

	std::vector<Test> tests;
	const char* currentTest;

	void log(const char* property, const char* value) const;
	void log(const char* property, float value) const;
	void log(const char* property, double value) const;
	void log(const char* property, bool value) const;
	void log(const char* property, uint32_t value) const;
	void log(const char* property, int32_t value) const;
	void log(const char* property, const std::string& value) const;

	static bool TCA9555Check();
	static bool BaseboardCheck();
	static bool AcceleroTest();
	static bool RTCTest();
	static bool Time1();
	static bool Time2();
	static bool CameraCheck();
	static bool BatteryCheck();
	static bool VoltReferenceCheck();
	static bool SpeakerMicCheck();
	static bool SPIFFSTest();
	static bool HWVersion();

	static constexpr int16_t BatVoltageMinimum = 3300;
	static constexpr float VoltReference = 2500;
	static constexpr float VoltReferenceTolerance = 100;

	static constexpr uint32_t CheckTimeout = 500;

	static constexpr esp_vfs_spiffs_conf_t spiffsConfig = {
			.base_path = "/spiffs",
			.partition_label = "storage",
			.max_files = 8,
			.format_if_mount_failed = false
	};

	static constexpr uint8_t ButtonCount = 6;
};

#endif //BUTTERBOT_FIRMWARE_JIGHWTEST_H
