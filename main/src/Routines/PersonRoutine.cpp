#include "PersonRoutine.h"
#include "Util/Roaming.h"
#include "States/BBStateMachine.h"
#include <Statics/ApplicationStatics.h>
#include <Core/Application.h>
#include <Services/Audio/Audio.h>
#include <Util/stdafx.h>
#include <cstdlib>
#include <cmath>
#include <BBData.h>
#include <Phrases.h>
#include <Util/ServiceLocator.h>
#include "Audio/SpeechAudioGen.h"
#include "Audio/SpeechAudioSource.h"
#include "Audio/SpeechGen.h"
#include "Services/BaseBoard.h"
#include "Services/MotionService.h"
#include "Services/Com.h"
#include "FaceDet.h"

DEFINE_LOG(PersonRoutine)

PersonRoutine::~PersonRoutine(){
	const Application* app = ApplicationStatics::getApplication();

	if(BaseBoard* bb = app->getService<BaseBoard>()){
		bb->stopMotors();
	}

	if(faceDetActive){
		if(FaceDet* fd = app->getService<FaceDet>()){
			fd->stop();
			sm->unbindRoutine(fd->OnDetect);
			fd->unloadModel();
		}
	}
}

Routine::TickingState PersonRoutine::tick(float deltaTime){
	const Application* app = ApplicationStatics::getApplication();

	if(phase != Phase::Init && phase != Phase::ProxCheck && proxInterrupted){
		CMF_LOG(PersonRoutine, LogLevel::Info, "Unsafe proximity state - interrupting");
		baseBoard->stopMotors();
		return TickingState::Done;
	}

	switch(phase){
		case Phase::Init: {
			baseBoard = app->getService<BaseBoard>();
			faceDet = app->getService<FaceDet>();
			MotionService* motionService = app->getService<MotionService>();

			if(baseBoard == nullptr || faceDet == nullptr || motionService == nullptr){
				CMF_LOG(PersonRoutine, LogLevel::Error, "Missing required service(s)");
				return TickingState::Done;
			}

			if(baseBoard->getChargingState() != ChargingState::Unplugged){
				CMF_LOG(PersonRoutine, LogLevel::Info, "Charging - skipping");
				return TickingState::Done;
			}

			if(!motionService->isUprightOnGround()){
				CMF_LOG(PersonRoutine, LogLevel::Info, "Not upright on the ground - skipping");
				return TickingState::Done;
			}

			const bool turnRight = rand() % 2;
			scanMotorLeft = turnRight ? RotateSpeed : -RotateSpeed;
			scanMotorRight = turnRight ? -RotateSpeed : RotateSpeed;

			sm->bindRoutine(baseBoard->onProximityReading, this, &PersonRoutine::onProximityReading);
			sm->bindRoutine(baseBoard->onProximityChange, this, &PersonRoutine::onProximityChange);
			baseBoard->requestProximityState();
			proxDeadlineMs = millis() + ProxReadingTimeoutMs;
			phase = Phase::ProxCheck;
			return TickingState::Continue;
		}

		case Phase::ProxCheck: {
			if(proxUnsafe || proxInterrupted){
				CMF_LOG(PersonRoutine, LogLevel::Info, "Unsafe proximity state - skipping");
				return TickingState::Done;
			}
			if(!proxReadingReceived && millis() < proxDeadlineMs){
				return TickingState::Continue;
			}

			faceDet->loadModel();
			sm->bindRoutine(faceDet->OnDetect, this, &PersonRoutine::onDetect);
			faceDet->start(false);
			faceDetActive = true;

			detected = false;
			scanDeadlineMs = millis() + (Roaming::enabled ? ScanWindowMs : StationaryScanWindowMs);
			phase = Phase::Scanning;
			return TickingState::Continue;
		}

		case Phase::Scanning: {
			if(detected){
				// Roaming off: greet from where it stands, no centering or driving
				if(!Roaming::enabled){
					greetKnown = detectedKnown;
					faceDet->stop();
					greet();
					return TickingState::Done;
				}
				beginApproach();
				return TickingState::Continue;
			}

			if(millis() >= scanDeadlineMs){
				if(!Roaming::enabled){
					CMF_LOG(PersonRoutine, LogLevel::Info, "No face ahead, roaming off - not turning");
					return TickingState::Done;
				}
				if(rotationCount >= MaxRotations){
					CMF_LOG(PersonRoutine, LogLevel::Info, "No face after full rotation");
					return TickingState::Done;
				}

				faceDet->stop();
				baseBoard->setMotors(scanMotorLeft, scanMotorRight);
				rotateDeadlineMs = millis() + Rotate45Ms;
				phase = Phase::Rotating;
			}

			return TickingState::Continue;
		}

		case Phase::Rotating: {
			if(millis() >= rotateDeadlineMs){
				baseBoard->stopMotors();
				rotationCount++;
				faceDet->start(false);
				detected = false;
				scanDeadlineMs = millis() + ScanWindowMs;
				phase = Phase::Scanning;
			}

			return TickingState::Continue;
		}

		case Phase::Centering: {
			const float error = faceCenterX - 0.5f;

			if(std::fabs(error) <= CenterDeadband || millis() >= centerDeadlineMs){
				baseBoard->stopMotors();
				driveEndMs = millis() + DriveMinMs + static_cast<uint32_t>(rand() % (DriveMaxMs - DriveMinMs + 1));
				baseBoard->setMotors(DriveSpeed, DriveSpeed);
				phase = Phase::Driving;
				return TickingState::Continue;
			}

			// A face on the right of the frame (error > 0) is to the robot's right, so turn right.
			if(error > 0.0f){
				baseBoard->setMotors(CenterSpeed, -CenterSpeed);
			} else{
				baseBoard->setMotors(-CenterSpeed, CenterSpeed);
			}

			return TickingState::Continue;
		}

		case Phase::Driving: {
			if(millis() >= driveEndMs){
				baseBoard->stopMotors();
				faceDet->stop();
				greet();
				return TickingState::Done;
			}

			return TickingState::Continue;
		}
	}

	return TickingState::Done;
}

void PersonRoutine::beginApproach(){
	greetKnown = detectedKnown;
	centerDeadlineMs = millis() + CenterTimeoutMs;
	phase = Phase::Centering;
}

void PersonRoutine::greet(){
	const Application* app = ApplicationStatics::getApplication();
	Audio* audio = app->getService<Audio>();
	Com* com = app->getService<Com>();

	if(audio == nullptr || ServiceLocator::SpeechAudioGenInstance == nullptr || ServiceLocator::SpeechGenInstance == nullptr || com == nullptr){
		CMF_LOG(PersonRoutine, LogLevel::Error, "Missing required service(s)");
		return;
	}

	const Phrase phrase = greetKnown ? Phrase::PersonOwnerGreeting : Phrase::PersonStrangerGreeting;
	const int16_t id = Phrases::get(phrase);
	if(id < 0){
		CMF_LOG(PersonRoutine, LogLevel::Warning, "Phrases::get returned no phrase id");
		return;
	}

	PersonData data{};
	data.id = id;
	data.greetKnown = greetKnown;
	com->sendData(BB::State::Idle, BB::Action::Idle::Person, data);

	auto source = std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, Phrases::map(phrase, id));
	audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::move(source));
	audio->waitEnd(portMAX_DELAY);
}

void PersonRoutine::onDetect(int faceCount, bool known, glm::vec2 pos, glm::vec2 size){
	if(faceCount <= 0){
		return;
	}

	detectedKnown = known;
	faceCenterX = pos.x + size.x * 0.5f;
	detected = true;
}

void PersonRoutine::onProximityReading(ProxState front, ProxState bottom){
	proxReadingReceived = true;
	proxUnsafe = (front == ProxState::Covered);
}

void PersonRoutine::onProximityChange(ProxSensor sensor, bool state){
	if(sensor == ProxSensor::Front && state){
		proxInterrupted = true;
	}
}
