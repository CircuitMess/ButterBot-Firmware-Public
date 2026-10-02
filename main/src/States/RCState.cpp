#include "RCState.h"
#include <Periphery/WiFi.h>
#include <Services/Audio/Audio.h>
#include <Services/Audio/AACAudioGenerator.h>
#include <Services/Audio/FileAudioSource.h>
#include <Services/ControllerState.h>
#include <Util/ServiceLocator.h>
#include "FaceDet.h"
#include "Audio/AudioFrontend.h"
#include "Services/Com.h"
#include "Util/WiFiStation.h"
#include "IdleState.h"
#include "Util/Feed.h"
#include "Services/LEDModuleControl.h"
#include "glm.hpp"
#include "gtx/vector_angle.inl"
#include "BBStateMachine.h"

DEFINE_LOG(RCState)

RCState::RCState(BBStateMachine* sm) : State(sm){
	const Application* app = Application::getApp();

	if(LEDModuleControl* leds = app->getService<LEDModuleControl>()){
		leds->turnLedOff();
	}

	Com* com = app->getService<Com>();
	if(com == nullptr){
		CMF_LOG(RCState, Warning, "Com is nullptr in postInitProperties.");
		return;
	}

	sm->bindState(com->OnCommand, this, &RCState::onCommand);
	sm->bindState(com->OnDriveData, this, &RCState::onDriveData);
	sm->bindState(com->OnConnStatus, this, &RCState::onBTDisconnect);

	wifi = newObject<WiFi>(sm);
	if(wifi == nullptr){
		CMF_LOG(RCState, Warning, "WiFi is nullptr in postInitProperties.");
		return;
	}

	if(!ServiceLocator::ControllerStateInstance){
		CMF_LOG(RCState, Warning, "ControllerState is nullptr in postInitProperties.");
		return;
	}

	wifiSta = newObject<WiFiStation>(sm, *wifi, ServiceLocator::ControllerStateInstance->getSSID(), ServiceLocator::ControllerStateInstance->getPassword());

	sm->bindState(wifiSta->OnStationEvent, this, &RCState::onWifiDisconnect);

	wifiSta->connect();

	feed = newObject<Feed>(sm);
}

RCState::~RCState(){
	if(BaseBoard* baseBoard = Application::getApp()->getService<BaseBoard>()){
		baseBoard->setMotors(0, 0);
	}

	delete *feed;
	delete *wifiSta;
	delete *wifi;
}

int64_t RCState::getDynamicTickInterval() const noexcept{
	if(feed == nullptr){
		return -1;
	}

	if(!feed->isActive()){
		return 1000;
	}

	return 30;
}

void RCState::onCommand(Ctrl::Command cmd){
	if(cmd == Ctrl::Command::ExitRC){
		sm->transitionTo<IdleState>();
		return;
	}

	if(cmd == Ctrl::Command::RCSound){
		playRandomSound();
		return;
	}
}

void RCState::playRandomSound(){
	const Application* app = Application::getApp();

	Audio* audio = app->getService<Audio>();
	if(audio == nullptr){
		CMF_LOG(RCState, Warning, "Audio is nullptr in playRandomSound.");
		return;
	}

	AACAudioGenerator* aacGen = app->getService<AACAudioGenerator>();
	if(aacGen == nullptr){
		CMF_LOG(RCState, Warning, "AACAudioGenerator is nullptr in playRandomSound.");
		return;
	}

	if(audio->isPlaying()){
		return;
	}

	static constexpr const char* Sounds[] = {
		"/spiffs/listen/lasers1/phaseJump1.aac",
		"/spiffs/listen/lasers1/phaseJump2.aac",
		"/spiffs/listen/lasers1/phaseJump3.aac",
		"/spiffs/listen/lasers1/phaseJump4.aac",
		"/spiffs/listen/lasers1/phaseJump5.aac",
		"/spiffs/listen/lasers1/phaserDown1.aac",
		"/spiffs/listen/lasers1/phaserDown2.aac",
		"/spiffs/listen/lasers1/phaserDown3.aac",
		"/spiffs/listen/lasers2/laser1.aac",
		"/spiffs/listen/lasers2/laser2.aac",
		"/spiffs/listen/lasers2/laser3.aac",
		"/spiffs/listen/lasers2/laser4.aac",
		"/spiffs/listen/lasers2/laser5.aac",
		"/spiffs/listen/lasers2/laser6.aac",
		"/spiffs/listen/lasers2/laser7.aac",
		"/spiffs/listen/lasers2/laser8.aac",
		"/spiffs/listen/lasers2/laser9.aac",
		"/spiffs/listen/robot/robot_1.aac",
		"/spiffs/listen/robot/robot_2.aac",
		"/spiffs/listen/robot/robot_3.aac",
		"/spiffs/listen/robot/robot_4.aac",
	};

	const char* path = Sounds[rand() % std::size(Sounds)];

	audio->play(aacGen, std::make_unique<FileAudioSource>(path));
}

void RCState::onDriveData(DriveData driveData){
	const Application* app = Application::getApp();

	BaseBoard* baseBoard = app->getService<BaseBoard>();
	if(baseBoard == nullptr){
		CMF_LOG(RCState, Warning, "BaseBoard is nullptr in event callback.");
		return;
	}

	if(driveData.joystickX == 0 && driveData.joystickY == 0){
		baseBoard->setMotors(driveData.joystickX, driveData.joystickY);
		return;
	}

	static constexpr float MaxSpeed = 100.0f;

	// Calculate motor speeds based on joystick positions
	glm::vec2 direction = { driveData.joystickX, driveData.joystickY };
	const float speed = glm::clamp(glm::length(direction), 0.0f, MaxSpeed);
	direction = glm::normalize(direction);

	float angle = glm::degrees(glm::angle(direction, { 0.0, 1.0 }));

	if(direction.x < 0){
		angle = 360.0f - angle;
	}

	static constexpr float circParts = 360.0f / 8.0f;

	float calcAngle = angle + circParts / 2.0f;
	if(calcAngle >= 360){
		calcAngle -= 360.0f;
	}
	const uint8_t number = std::floor(calcAngle / circParts);

	float leftSpeed = 0.0f;
	float rightSpeed = 0.0f;

	if(number == 0){
		leftSpeed = rightSpeed = 1.0f;
	}
	else if(number == 1){
		leftSpeed = 1.0f;
		rightSpeed = 0.2f;
	}
	else if(number == 2){
		leftSpeed = 1.0f;
		rightSpeed = -1.0f;
	}
	else if(number == 3){
		leftSpeed = -1.0f;
		rightSpeed = -0.2f;
	}
	else if(number == 4){
		leftSpeed = rightSpeed = -1.0f;
	}
	else if(number == 5){
		leftSpeed = -0.2f;
		rightSpeed = -1.0f;
	}
	else if(number == 6){
		leftSpeed = -1.0f;
		rightSpeed = 1.0f;
	}
	else if(number == 7){
		leftSpeed = 0.2f;
		rightSpeed = 1.0f;
	}

	leftSpeed = std::clamp(leftSpeed * speed * 2.0f, -100.0f, 100.0f);
	rightSpeed = std::clamp(rightSpeed * speed * 2.0f, -100.0f, 100.0f);

	baseBoard->setMotors(leftSpeed, rightSpeed);
}

void RCState::onBTDisconnect(Com::ConnStatus status){
	if(status != Com::ConnStatus::Disconnected){
		return;
	}

	sm->transitionTo<IdleState>();
}

void RCState::onWifiDisconnect(WiFiStation::EventType type, bool success){
	if(type != WiFiStation::EventType::Disconnect){
		return;
	}

	sm->transitionTo<IdleState>();
}
