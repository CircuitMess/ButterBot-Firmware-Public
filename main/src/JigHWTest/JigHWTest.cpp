#include "JigHWTest.h"
#include "SPIFFSChecksum.hpp"
#include <Pins.hpp>
#include <esp_efuse.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <esp_mac.h>
#include <Battery/BattReaderADC.h>
#include <Devices/Camera.h>
#include "Devices/SC7A20.h"
#include <Drivers/Output/OutputGPIO.h>
#include <Drivers/Output/OutputTCA.h>
#include <Services/ADCReader.h>
#include "Util/EfuseMeta.h"
#include <Memory/ObjectMemory.h>
#include <Periphery/ADCUnit.h>
#include <Periphery/I2S.h>
#include <Services/ISRButtonInput.h>
#include <Services/Audio/AACAudioGenerator.h>
#include <Services/Audio/Audio.h>
#include <Services/Audio/FileAudioSource.h>
#include <Util/HardwareConfiguration.h>
#include <bootloader_random.h>
#include <esp_random.h>
#include "Services/ShutdownService.h"

JigHWTest::JigHWTest(){
	bootloader_random_enable();
	srand(esp_random());
	bootloader_random_disable();

	i2cMain = newObject<I2CMaster>(this, I2CPort::Zero, static_cast<gpio_num_t>(I2C_MAIN_SDA), static_cast<gpio_num_t>(I2C_MAIN_SCL));
	if(i2cMain == nullptr){
		printf("TEST:fail:Main I2C in null\n");
		abort();
	}

	test = this;

	tests.push_back({ TCA9555Check, "TCA9555 check", [](){}});
	tests.push_back({ BaseboardCheck, "Baseboard check", [](){}});
	tests.push_back({ AcceleroTest, "Accelerometer check", [](){}});
	tests.push_back({ RTCTest, "RTC check", [](){}});
	tests.push_back({ Time1, "RTC crystal 1", [](){}});
	tests.push_back({ Time2, "RTC crystal 2", [](){}});
	tests.push_back({ CameraCheck, "Camera check", [](){}});
	tests.push_back({ BatteryCheck, "Battery check", [](){}});
	tests.push_back({ VoltReferenceCheck, "Voltage ref", [](){ tca->write(EXP_CALIB_EN, false); }});
	tests.push_back({ SpeakerMicCheck, "Speaker and mic", [](){ tca->write(EXP_SD_MODE_PIN, false); }});
	tests.push_back({ SPIFFSTest, "SPIFFS check", [](){}});
	tests.push_back({ HWVersion, "HW rev", [](){}});
}

bool JigHWTest::checkJig(){
	char buf[7];
	int wp = 0;

	const uint32_t start = millis();
	int c;
	while(millis() - start < CheckTimeout){
		vTaskDelay(1);
		c = getchar();
		if(c == EOF) continue;
		buf[wp] = static_cast<char>(c);
		wp = (wp + 1) % 7;

		for(int i = 0; i < 7; i++){
			int match = 0;
			static const char* target = "JIGTEST";

			for(int j = 0; j < 7; j++){
				match += buf[(i + j) % 7] == target[j];
			}

			if(match == 7){
				// This is important, desktop app freezes otherwise if the UART/JTAG buffer isn't emptied when it tries to write something again
				while(int c2 = getchar() != EOF) {}

				return true;
			}
		}
	}

	return false;
}


void JigHWTest::start(){
	uint64_t _chipmacid = 0LL;
	esp_efuse_mac_get_default((uint8_t*) (&_chipmacid));
	printf("\nTEST:begin:%llx\n", _chipmacid);

	bool pass = true;
	for(const Test& test : tests){
		currentTest = test.name;

		printf("TEST:startTest:%s\n", currentTest);

		const bool result = test.test();

		printf("TEST:endTest:%s\n", result ? "pass" : "fail");

		if(!(pass &= result)){
			if(test.onFail){
				test.onFail();
			}

			break;
		}
	}

	if(pass){
		printf("TEST:passall\n");
	}else{
		printf("TEST:fail:%s\n", currentTest);
		return;
	}


	//------------------------------------------------------
	bool painted = false;
	bool drive = false;

	StrongObjectPtr<GPIOPeriph> gpio = newObject<GPIOPeriph>();
	StrongObjectPtr<OutputGPIO> gpioOut = newObject<OutputGPIO>(nullptr, HardwareConfiguration::getGpioOutputPins(), gpio);
	StrongObjectPtr<InputGPIO> gpioInput = newObject<InputGPIO>(nullptr, HardwareConfiguration::getGpioInputPins(), gpio);
	StrongObjectPtr<I2S> i2s = newObject<I2S>(nullptr, I2S_NUM_AUTO, HardwareConfiguration::getAudioI2SConfig());
	StrongObjectPtr<OutputTCA> tcaOutput = newObject<OutputTCA>(nullptr, HardwareConfiguration::getTcaOutputPins(), *tca);
	StrongObjectPtr<Audio> audio = newObject<Audio>(nullptr, *i2s, OutputPin{*tcaOutput, EXP_SD_MODE_PIN});
	audio->setGain(1.0f);

	tca->pinMode(EXP_PIN_LED, TCA9555::PinMode::OUT);

	StrongObjectPtr<AACAudioGenerator> generator = newObject<AACAudioGenerator>();

	std::unique_ptr<I2CDevice> baseDevice = i2cMain->addDevice(HardwareConfiguration::getBaseBoardAddress());
	StrongObjectPtr<BaseBoard> baseBoard = newObject<BaseBoard>(nullptr, std::move(baseDevice), /*internalStack=*/false);
	StrongObjectPtr<ISRButtonInput> input = newObject<ISRButtonInput>(nullptr, (int)Button::Power, InputPin{*gpioInput, PIN_BTN}, /*internalStack=*/false);

	static constexpr uint64_t PowerOffHoldTime = 5000;
	bool pressed = false;
	uint64_t pressStart = 0;

	for(;;){
		if(input->getState()){
			if(!pressed){
				pressed = true;
				pressStart = millis();
			}else if(millis() - pressStart >= PowerOffHoldTime){
				baseBoard->stopMotors();
				tca->write(EXP_PIN_LED, false);
				ShutdownService::PowerOff();
			}

			baseBoard->setMotors(100, 100);
		}else{
			pressed = false;
			baseBoard->stopMotors();
		}

		if(millis() % 2000 <= 1000){
			if(!painted){
				// audio->play(*generator, std::make_unique<FileAudioSource>("/spiffs/listen/robot/robot_1.aac"));
				// audio->enqueue(*generator, std::make_unique<FileAudioSource>("/spiffs/listen/robot/robot_2.aac"));
				// audio->enqueue(*generator, std::make_unique<FileAudioSource>("/spiffs/listen/robot/robot_3.aac"));
				// audio->enqueue(*generator, std::make_unique<FileAudioSource>("/spiffs/listen/robot/robot_4.aac"));
				tca->write(EXP_PIN_LED, true);
				painted = true;
			}
		}else{
			if(painted){
				tca->write(EXP_PIN_LED, false);
				painted = false;
			}
		}
	}
}

void JigHWTest::log(const char* property, const char* value) const {
	printf("%s:%s:%s\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, float value) const {
	printf("%s:%s:%f\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, double value) const {
	printf("%s:%s:%lf\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, bool value) const {
	printf("%s:%s:%s\n", currentTest, property, value ? "TRUE" : "FALSE");
}

void JigHWTest::log(const char* property, uint32_t value) const {
	printf("%s:%s:%lu\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, int32_t value) const {
	printf("%s:%s:%ld\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, const std::string& value) const {
	printf("%s:%s:%s\n", currentTest, property, value.c_str());
}

bool JigHWTest::TCA9555Check(){
	if(i2cMain->probe(HardwareConfiguration::getTcaAddress(), 200) != ESP_OK){
		return false;
	}

	tca = newObject<TCA9555>(nullptr, *i2cMain, HardwareConfiguration::getTcaAddress());

	return true;
}

bool JigHWTest::BaseboardCheck(){
	if(i2cMain->probe(HardwareConfiguration::getBaseBoardAddress(), 200) != ESP_OK){
		return false;
	}

	// Identify register is expected to return the BaseBoard's own I2C address (see BaseBoard's constructor)
	static constexpr uint8_t IdentifyReg = 0x0;

	const std::unique_ptr<I2CDevice> dev = i2cMain->addDevice(HardwareConfiguration::getBaseBoardAddress());
	if(dev == nullptr){
		test->log("addDevice", "failed");
		return false;
	}

	uint8_t id = 0;
	if(dev->readRegister(IdentifyReg, id, 200 / portTICK_PERIOD_MS) != ESP_OK){
		test->log("identify", "read failed");
		return false;
	}

	if(id != HardwareConfiguration::getBaseBoardAddress()){
		test->log("identify", (uint32_t) id);
		return false;
	}

	return true;
}

bool JigHWTest::AcceleroTest(){
	if(i2cMain->probe(HardwareConfiguration::getAcceleroAddress(), 200) != ESP_OK){
		return false;
	}

	const std::unique_ptr<I2CDevice> dev = i2cMain->addDevice(HardwareConfiguration::getAcceleroAddress());
	if(dev == nullptr){
		test->log("addDevice", "failed");
		return false;
	}

	uint8_t id = 0;
	if(dev->readRegister(SC7A20::WhoAmIReg, id, 200 / portTICK_PERIOD_MS) != ESP_OK){
		test->log("WHO_AM_I", "read failed");
		return false;
	}

	if(id != SC7A20::SC7A20_ID){
		test->log("WHO_AM_I", (uint32_t) id);
		return false;
	}

	return true;
}

bool JigHWTest::RTCTest(){
	if(i2cMain->probe(HardwareConfiguration::getRTCAddress(), 200) != ESP_OK){
		return false;
	}

	rtc = newObject<BM8563>(nullptr, *i2cMain, HardwareConfiguration::getRTCAddress());

	return true;
}

bool JigHWTest::Time1(){
	tm t = rtc->getTime();
	const time_t unixt = mktime(&t);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	tm t2 = rtc->getTime();
	const time_t unixt2 = mktime(&t2);
	const double diff = std::difftime(unixt2, unixt);

	if(diff != 1 && diff != 2){
		test->log("time passage (expected 1s or 2s)", (uint32_t) diff);
		return false;
	}

	return true;
}

bool JigHWTest::Time2(){
	static constexpr size_t count = 1000;

	time_t lastTime = 0;

	for(uint32_t i = 0; i < count; i++){

		tm t = rtc->getTime();
		const time_t unixt = mktime(&t);

		if(i == 0){
			lastTime = unixt;
			continue;
		}
		vTaskDelay(1);

		const double diff = abs(difftime(unixt, lastTime));
		if(diff > 1){
			test->log("reading", i);
			test->log("diff", diff);
			return false;
		}

		lastTime = unixt;
	}
	return true;
}

bool JigHWTest::CameraCheck(){
	StrongObjectPtr<Camera> camera = newObject<Camera>(nullptr, HardwareConfiguration::getCameraConfig(static_cast<int>(i2cMain->getPort())), *i2cMain, [](sensor_t* sensor){
			sensor->set_hmirror(sensor, 0);
			sensor->set_vflip(sensor, 0);
			sensor->set_gain_ctrl(sensor, 1);
		});

	if(camera->init() != ESP_OK){
		return false;
	}

	static constexpr uint32_t FrameCount = 3;
	for(uint32_t i = 0; i < FrameCount; i++){
		camera_fb_t* frame = camera->getFrame();
		if(frame == nullptr || frame->len == 0){
			test->log("frame", i);
			return false;
		}
		camera->releaseFrame();
	}

	return true;
}

bool JigHWTest::BatteryCheck(){
	//Just in case BattVref was active
	tca->pinMode(EXP_CALIB_EN, TCA9555::PinMode::OUT);
	tca->write(EXP_CALIB_EN, false);
	delayMillis(100);

	constexpr adc_oneshot_chan_cfg_t cfg = {
		.atten = ADC_ATTEN_DB_2_5,
		.bitwidth = ADC_BITWIDTH_12
	};

	static constexpr float Factor = 4.0f;
	static constexpr float Offset = 0;

	const StrongObjectPtr<ADCReader> reader = newObject<ADCReader>(nullptr, (gpio_num_t) PIN_BATT, cfg, true, newObject<FactorOffset_ADCFilter>(nullptr, Factor, Offset).get());

	static constexpr uint16_t numReadings = 50;
	static constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		reading += reader->sample();
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	test->log("reading", reading);

	if(reading < BatVoltageMinimum){
		return false;
	}

	return true;
}

bool JigHWTest::VoltReferenceCheck(){
	tca->pinMode(EXP_CALIB_EN, TCA9555::PinMode::OUT);
	tca->write(EXP_CALIB_EN, true);
	delayMillis(100);

	constexpr adc_oneshot_chan_cfg_t cfg = {
		.atten = ADC_ATTEN_DB_2_5,
		.bitwidth = ADC_BITWIDTH_12
	};

	static constexpr float Factor = 4.0f;
	static constexpr float Offset = 0;

	const StrongObjectPtr<ADCReader> reader = newObject<ADCReader>(nullptr, (gpio_num_t) PIN_BATT, cfg, true, newObject<FactorOffset_ADCFilter>(nullptr, Factor, Offset).get());

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		reading += reader->sample();
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	test->log("reading", reading);

	if(reading < VoltReference - VoltReferenceTolerance || reading > VoltReference + VoltReferenceTolerance){
		return false;
	}

	tca->write(EXP_CALIB_EN, false);

	return true;
}

static float goertzelMag(const std::vector<int16_t>& samples, uint32_t freq, uint32_t sampleRate){
	const float w = 2.0f * (float) M_PI * (float) freq / (float) sampleRate;
	const float coeff = 2.0f * cosf(w);
	float q1 = 0;
	float q2 = 0;

	for(int16_t sample : samples){
		const float q0 = coeff * q1 - q2 + (float) sample / 32768.0f;
		q2 = q1;
		q1 = q0;
	}

	return sqrtf(q1 * q1 + q2 * q2 - coeff * q1 * q2);
}

bool JigHWTest::SpeakerMicCheck(){
	static constexpr uint32_t SampleRate = 16000;
	static constexpr uint32_t FreqMin = 600;
	static constexpr uint32_t FreqMax = 1200;
	static constexpr uint32_t FreqStep = 10;
	static constexpr uint32_t MinFreqSpacing = 100;
	static constexpr uint32_t HarmonicGuard = 30;
	static constexpr uint32_t FreqTolerance = 20;
	static constexpr uint32_t PeakExclusion = 30;
	static constexpr size_t LoopSamples = 1600;
	static constexpr size_t CaptureSamples = 12800;
	static constexpr size_t DiscardFrames = 4800;
	static constexpr float MinSNR = 5.0f;

	const uint32_t freqCount = (FreqMax - FreqMin) / FreqStep + 1;
	const uint32_t freq1 = FreqMin + FreqStep * (rand() % freqCount);
	uint32_t freq2 = freq1;
	while(abs((int32_t) freq2 - (int32_t) freq1) < (int32_t) MinFreqSpacing ||
		  abs((int32_t) freq2 - (int32_t) (2 * freq1)) < (int32_t) HarmonicGuard ||
		  abs((int32_t) (2 * freq2) - (int32_t) freq1) < (int32_t) HarmonicGuard){
		freq2 = FreqMin + FreqStep * (rand() % freqCount);
	}

	test->log("freq1", freq1);
	test->log("freq2", freq2);

	std::vector<int16_t> loopBuf(LoopSamples);
	for(size_t i = 0; i < LoopSamples; i++){
		const float t = (float) i / (float) SampleRate;
		const float sample = 0.1225f * sinf(2.0f * (float) M_PI * (float) freq1 * t) + 0.1225f * sinf(2.0f * (float) M_PI * (float) freq2 * t);
		loopBuf[i] = (int16_t) (sample * 32767.0f);
	}

	StrongObjectPtr<I2S> i2sOut = newObject<I2S>(nullptr, I2S_NUM_1, HardwareConfiguration::getAudioI2SConfig());
	StrongObjectPtr<I2S> i2sIn = newObject<I2S>(nullptr, I2S_NUM_0, HardwareConfiguration::getMicI2SConfig());

	static constexpr uint32_t DelayTimes[] = {500, 1000, 1500, 2000};
	uint32_t delay = DelayTimes[rand() % std::size(DelayTimes)];
	test->log("delay", delay);

	delayMillis(delay);

	tca->pinMode(EXP_SD_MODE_PIN, TCA9555::PinMode::OUT);
	tca->write(EXP_SD_MODE_PIN, true);

	struct PlaybackCtx {
		I2S* i2s;
		int16_t* data;
		size_t bytes;
		volatile bool stop;
		volatile bool done;
	} ctx = { i2sOut.get(), loopBuf.data(), LoopSamples * sizeof(int16_t), false, false };

	const BaseType_t taskCreated = xTaskCreate([](void* arg){
		PlaybackCtx* ctx = (PlaybackCtx*) arg;
		while(!ctx->stop){
			ctx->i2s->write((uint8_t*) ctx->data, ctx->bytes);
		}
		ctx->done = true;
		vTaskDelete(nullptr);
	}, "SpkMicTest", 3072, &ctx, 6, nullptr);

	if(taskCreated != pdPASS){
		tca->write(EXP_SD_MODE_PIN, false);
		delete i2sOut.get();
		delete i2sIn.get();
		test->log("playback task", "creation failed");
		return false;
	}

	std::vector<int16_t> stereoBuf(2048);
	bool readFail = false;

	size_t discarded = 0;
	while(discarded < DiscardFrames){
		const size_t frames = i2sIn->read((uint8_t*) stereoBuf.data(), stereoBuf.size() * sizeof(int16_t)) / (2 * sizeof(int16_t));
		if(frames == 0){
			readFail = true;
			break;
		}
		discarded += frames;
	}

	std::vector<int16_t> captureL(CaptureSamples);
	std::vector<int16_t> captureR(CaptureSamples);
	size_t captured = 0;
	while(!readFail && captured < CaptureSamples){
		const size_t frames = i2sIn->read((uint8_t*) stereoBuf.data(), stereoBuf.size() * sizeof(int16_t)) / (2 * sizeof(int16_t));
		if(frames == 0){
			readFail = true;
			break;
		}
		for(size_t i = 0; i < frames && captured < CaptureSamples; i++){
			captureL[captured] = stereoBuf[i * 2];
			captureR[captured] = stereoBuf[i * 2 + 1];
			captured++;
		}
	}

	ctx.stop = true;
	while(!ctx.done){
		vTaskDelay(1);
	}

	tca->write(EXP_SD_MODE_PIN, false);
	delete i2sOut.get();
	delete i2sIn.get();

	if(readFail){
		test->log("mic", "read failed");
		return false;
	}

	const std::pair<const char*, const std::vector<int16_t>*> channels[] = {
		{ "left", &captureL },
		{ "right", &captureR },
	};

	for(const std::pair<const char*, const std::vector<int16_t>*>& channel : channels){
		std::vector<float> mags(freqCount);
		for(uint32_t i = 0; i < freqCount; i++){
			mags[i] = goertzelMag(*channel.second, FreqMin + i * FreqStep, SampleRate);
		}

		uint32_t peakIndex1 = 0;
		for(uint32_t i = 1; i < freqCount; i++){
			if(mags[i] > mags[peakIndex1]){
				peakIndex1 = i;
			}
		}

		uint32_t peakIndex2 = freqCount;
		for(uint32_t i = 0; i < freqCount; i++){
			if((uint32_t) abs((int32_t) i - (int32_t) peakIndex1) * FreqStep <= PeakExclusion){
				continue;
			}
			if(peakIndex2 == freqCount || mags[i] > mags[peakIndex2]){
				peakIndex2 = i;
			}
		}

		float noiseSum = 0;
		uint32_t noiseCount = 0;
		for(uint32_t i = 0; i < freqCount; i++){
			if((uint32_t) abs((int32_t) i - (int32_t) peakIndex1) * FreqStep <= PeakExclusion ||
			   (uint32_t) abs((int32_t) i - (int32_t) peakIndex2) * FreqStep <= PeakExclusion){
				continue;
			}
			noiseSum += mags[i];
			noiseCount++;
		}
		const float noiseFloor = noiseSum / (float) noiseCount;

		const uint32_t detected1 = FreqMin + peakIndex1 * FreqStep;
		const uint32_t detected2 = FreqMin + peakIndex2 * FreqStep;

		char prop[32];
		snprintf(prop, sizeof(prop), "%s detected1", channel.first);
		test->log(prop, detected1);
		snprintf(prop, sizeof(prop), "%s detected2", channel.first);
		test->log(prop, detected2);
		snprintf(prop, sizeof(prop), "%s mag1", channel.first);
		test->log(prop, mags[peakIndex1]);
		snprintf(prop, sizeof(prop), "%s mag2", channel.first);
		test->log(prop, mags[peakIndex2]);
		snprintf(prop, sizeof(prop), "%s noiseFloor", channel.first);
		test->log(prop, noiseFloor);

		if(noiseFloor <= 0.0f || mags[peakIndex1] < MinSNR * noiseFloor || mags[peakIndex2] < MinSNR * noiseFloor){
			return false;
		}

		const bool directMatch = (uint32_t) abs((int32_t) detected1 - (int32_t) freq1) <= FreqTolerance && (uint32_t) abs((int32_t) detected2 - (int32_t) freq2) <= FreqTolerance;
		const bool crossMatch = (uint32_t) abs((int32_t) detected1 - (int32_t) freq2) <= FreqTolerance && (uint32_t) abs((int32_t) detected2 - (int32_t) freq1) <= FreqTolerance;

		if(!directMatch && !crossMatch){
			return false;
		}
	}

	return true;
}

bool JigHWTest::SPIFFSTest(){
	for(const auto& f : SPIFFSFiles){
		FILE* file = fopen(f.name, "rb");
		if(file == nullptr){
			test->log("missing", f.name);
			return false;
		}

		fseek(file, 0, SEEK_END);
		const long size = ftell(file);
		fclose(file);

		if(size < 0 || (size_t) size != f.size){
			test->log("size mismatch", f.name);
			test->log("expected", (uint32_t) f.size);
			test->log("got", (int32_t) size);
			return false;
		}
	}

	return true;
}

bool JigHWTest::HWVersion(){
	uint16_t version = 0;
	bool result = EfuseMeta::readPID(version);

	if(!result){
		test->log("HW version", "couldn't PID read from efuse");
		return false;
	}

	if(version != 0){
		test->log("Existing HW version", static_cast<uint32_t>(version));

		if(version == EfuseMeta::getHardcodedPID()){
			test->log("Already fused.", static_cast<uint32_t>(version));
			return true;
		}else{
			test->log("Wrong binary already fused!", static_cast<uint32_t>(version));
			return false;
		}
	}

	return EfuseMeta::write();
}
