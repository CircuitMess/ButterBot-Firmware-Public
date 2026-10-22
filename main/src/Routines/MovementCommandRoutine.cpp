#include "MovementCommandRoutine.h"
#include <Statics/ApplicationStatics.h>
#include <Util/stdafx.h>
#include <Core/Application.h>
#include <Services/Audio/Audio.h>
#include <Util/ServiceLocator.h>
#include "Audio/SpeechAudioGen.h"
#include "Audio/SpeechAudioSource.h"
#include "Audio/SpeechGen.h"
#include "States/BBStateMachine.h"
#include "Services/Com.h"
#include "Services/MotionService.h"

MovementCommandRoutine::~MovementCommandRoutine(){
	if(baseboard != nullptr){
		baseboard->stopMotors();
	}
}

Routine::TickingState MovementCommandRoutine::tick(float deltaTime){
	const Application* app = ApplicationStatics::getApplication();

	if(phase == Phase::Init){
		baseboard = app->getService<BaseBoard>();
		com = app->getService<Com>();
		MotionService* motion = app->getService<MotionService>();

		if(baseboard == nullptr || motion == nullptr){
			ESP_LOGE("MovementCommandRoutine", "Missing required service(s)");
			abort();
		}

		if(baseboard->getChargingState() != ChargingState::Unplugged){
			com->sendData(BB::State::Idle, BB::Action::Idle::CantMove, CantMoveData{.reason = CantMoveData::Reason::Charging});
			speak(Phrase::CannotPerformPlugged);
			return TickingState::Done;
		}

		if(!motion->isUprightOnGround()){
			com->sendData(BB::State::Idle, BB::Action::Idle::CantMove, CantMoveData{.reason = CantMoveData::Reason::Unstable});
			speak(Phrase::CannotMove);
			return TickingState::Done;
		}

		sm->bindRoutine(baseboard->onProximityReading, this, &MovementCommandRoutine::onProximityReading);
		sm->bindRoutine(baseboard->onProximityChange, this, &MovementCommandRoutine::onProximityChange);
		if(ServiceLocator::SettingsInstance && ServiceLocator::SettingsInstance->get().bottomSensor == BottomSensorMode::On){
			sm->bindRoutine(motion->onMotion, this, &MovementCommandRoutine::onMotion);
		}
		baseboard->requestProximityState();
		proxDeadlineMs = millis() + ProxReadingTimeoutMs;

		phase = Phase::ProxCheck;
		return TickingState::Continue;
	}

	if(phase == Phase::ProxCheck){
		if(proxUnsafe || proxInterrupted){
			speak(Phrase::CannotMove);
			return TickingState::Done;
		}
		if(!proxReadingReceived && millis() < proxDeadlineMs){
			return TickingState::Continue;
		}

		Audio* audio = app->getService<Audio>();
		Com* com = app->getService<Com>();
		MotionService* motion = app->getService<MotionService>();

		if(audio == nullptr || !ServiceLocator::SpeechAudioGenInstance || !ServiceLocator::SpeechGenInstance || com == nullptr || baseboard == nullptr || motion == nullptr){
			ESP_LOGE("MovementCommandRoutine", "Missing required service(s)");
			abort();
		}

		const MovementCommand command = getMovementCommand();

		const int16_t id = Phrases::get(command.phrase);
		if(id < 0) return TickingState::Done;

		VoiceControlData data{};
		data.direction = command.direction;
		com->sendData(BB::State::Scenario, BB::Action::Scenario::VoiceControl, data);

		auto source = std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, Phrases::map(command.phrase, id));
		audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::move(source));
		audio->waitEnd(portMAX_DELAY);

		const bool bottomSensorOn = ServiceLocator::SettingsInstance && ServiceLocator::SettingsInstance->get().bottomSensor == BottomSensorMode::On;
		if(!bottomSensorOn && !motion->isUprightOnGround()){
			speak(Phrase::CannotMove);
			return TickingState::Done;
		}

		baseboard->setMotors(command.leftSpeed, command.rightSpeed);
		moveEndMs = millis() + command.duration;
		phase = Phase::Moving;
		return TickingState::Continue;
	}

	if(phase == Phase::Moving){
		if(proxInterrupted){
			baseboard->stopMotors();
			speak(Phrase::RoutineInterrupted);
			return TickingState::Done;
		}

		if(millis() >= moveEndMs){
			baseboard->stopMotors();
			return TickingState::Done;
		}

		return TickingState::Continue;
	}

	return TickingState::Done;
}

void MovementCommandRoutine::speak(Phrase phrase){
	const Application* app = ApplicationStatics::getApplication();
	Audio* audio = app->getService<Audio>();

	if(audio == nullptr || !ServiceLocator::SpeechAudioGenInstance || !ServiceLocator::SpeechGenInstance){
		ESP_LOGE("MovementCommandRoutine", "Missing required service(s)");
		return;
	}

	const int16_t id = Phrases::get(phrase);
	if(id < 0) return;

	auto source = std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, Phrases::map(phrase, id));
	audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::move(source));
	audio->waitEnd(portMAX_DELAY);
}

void MovementCommandRoutine::onProximityReading(ProxState front, ProxState bottom){
	proxReadingReceived = true;
	if(ServiceLocator::SettingsInstance && ServiceLocator::SettingsInstance->get().bottomSensor == BottomSensorMode::On){
		proxUnsafe = (bottom == ProxState::Uncovered) || (front == ProxState::Covered);
	} else{
		proxUnsafe = (front == ProxState::Covered);
	}
}

void MovementCommandRoutine::onProximityChange(ProxSensor sensor, bool state){
	if(ServiceLocator::SettingsInstance && ServiceLocator::SettingsInstance->get().bottomSensor == BottomSensorMode::On){
		if((sensor == ProxSensor::Bottom && !state) || (sensor == ProxSensor::Front && state)){
			proxInterrupted = true;
		}
	} else{
		if(sensor == ProxSensor::Front && state){
			proxInterrupted = true;
		}
	}
}

void MovementCommandRoutine::onMotion(MotionType type){
	proxInterrupted = true;
}
