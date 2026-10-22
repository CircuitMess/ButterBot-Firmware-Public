#include "WanderRoutine.h"
#include <cstdlib>
#include <Util/stdafx.h>
#include <Core/Application.h>
#include <Statics/ApplicationStatics.h>
#include <BBData.h>
#include <Util/ServiceLocator.h>
#include "States/BBStateMachine.h"
#include "Services/MotionService.h"
#include "Services/Com.h"

DEFINE_LOG(WanderRoutine)

WanderRoutine::~WanderRoutine(){
	if(baseBoard){
		baseBoard->stopMotors();
	}
}

Routine::TickingState WanderRoutine::tick(float deltaTime){
	if(phase == Phase::Init){
		const Application* app = ApplicationStatics::getApplication();
		baseBoard = app->getService<BaseBoard>();
		MotionService* motionService = app->getService<MotionService>();
		Com* com = app->getService<Com>();

		if(baseBoard == nullptr || motionService == nullptr || com == nullptr){
			CMF_LOG(WanderRoutine, LogLevel::Error, "Missing required service(s)");
			return TickingState::Done;
		}

		// Don't wander while docked/charging.
		if(baseBoard->getChargingState() != ChargingState::Unplugged){
			return TickingState::Done;
		}

		// Only wander when upright and on the ground.
		if(!motionService->isUprightOnGround()){
			return TickingState::Done;
		}

		sm->bindRoutine(baseBoard->onProximityReading, this, &WanderRoutine::onProximityReading);
		sm->bindRoutine(baseBoard->onProximityChange, this, &WanderRoutine::onProximityChange);
		baseBoard->requestProximityState();
		proxDeadlineMs = millis() + ProxReadingTimeoutMs;
		phase = Phase::ProxCheck;
		return TickingState::Continue;
	}

	if(phase == Phase::ProxCheck){
		if(proxUnsafe || proxInterrupted){
			CMF_LOG(WanderRoutine, LogLevel::Info, "Unsafe proximity state - skipping");
			return TickingState::Done;
		}
		if(!proxReadingReceived && millis() < proxDeadlineMs){
			return TickingState::Continue;
		}

		// One rotation and one forward movement, in a random order.
		const bool rotateFirst = rand() % 2;
		actions[rotateFirst ? 0 : 1] = makeRotation();
		actions[rotateFirst ? 1 : 0] = makeForward();

		Com* com = ApplicationStatics::getApplication()->getService<Com>();
		com->sendData(BB::State::Idle, BB::Action::Idle::Wander, WanderData{});

		actionIndex = 0;
		startAction(actions[actionIndex]);
		phase = Phase::Moving;
		return TickingState::Continue;
	}

	if(phase == Phase::Moving){
		if(proxInterrupted){
			baseBoard->stopMotors();
			return TickingState::Done;
		}

		if(millis() < actionEndMs){
			return TickingState::Continue;
		}

		actionIndex++;
		if(actionIndex < 2){
			startAction(actions[actionIndex]);
			return TickingState::Continue;
		}

		baseBoard->stopMotors();
		phase = Phase::Done;
		return TickingState::Done;
	}

	return TickingState::Done;
}

WanderRoutine::Action WanderRoutine::makeRotation(){
	const uint16_t degrees = MinRotationDeg + static_cast<uint16_t>(rand() % (MaxRotationDeg - MinRotationDeg + 1));
	const uint32_t durationMs = static_cast<uint32_t>(degrees) * Rotation180Ms / Rotation180Deg;
	const bool turnRight = rand() % 2;

	return {
		static_cast<int8_t>(turnRight ? MotorSpeed : -MotorSpeed),
		static_cast<int8_t>(turnRight ? -MotorSpeed : MotorSpeed),
		durationMs
	};
}

WanderRoutine::Action WanderRoutine::makeForward(){
	const uint32_t durationMs = MinForwardMs + static_cast<uint32_t>(rand() % (MaxForwardMs - MinForwardMs + 1));
	return { MotorSpeed, MotorSpeed, durationMs };
}

void WanderRoutine::startAction(const Action& action){
	baseBoard->setMotors(action.left, action.right);
	actionEndMs = millis() + action.durationMs;
}

void WanderRoutine::onProximityReading(ProxState front, ProxState bottom){
	proxReadingReceived = true;
	if(ServiceLocator::SettingsInstance && ServiceLocator::SettingsInstance->get().bottomSensor == BottomSensorMode::On){
		proxUnsafe = (bottom == ProxState::Uncovered) || (front == ProxState::Covered);
	} else{
		proxUnsafe = (front == ProxState::Covered);
	}
}

void WanderRoutine::onProximityChange(ProxSensor sensor, bool state){
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
