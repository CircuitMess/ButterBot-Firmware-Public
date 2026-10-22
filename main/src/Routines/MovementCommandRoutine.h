#ifndef BUTTERBOT_FIRMWARE_MOVEMENTCOMMANDROUTINE_H
#define BUTTERBOT_FIRMWARE_MOVEMENTCOMMANDROUTINE_H

#include <BBData.h>
#include <Phrases.h>

#include "Routine.h"
#include "Services/BaseBoard.h"
#include "Services/MotionService.h"
#include "Services/Com.h"

class MovementCommandRoutine : public Routine {
public:
	using Routine::Routine;

	~MovementCommandRoutine() override;
	TickingState tick(float deltaTime) override;

protected:
	struct MovementCommand {
		Phrase phrase;
		VoiceControlData::Direction direction;
		int8_t leftSpeed;
		int8_t rightSpeed;
		uint32_t duration;
	};

	virtual MovementCommand getMovementCommand() const = 0;

private:
	void onProximityReading(ProxState front, ProxState bottom);
	void onProximityChange(ProxSensor sensor, bool state);
	void onMotion(MotionType type);
	void speak(Phrase phrase);

	enum class Phase : uint8_t { Init, ProxCheck, Moving };

	static constexpr uint32_t ProxReadingTimeoutMs = 500;

	Phase phase = Phase::Init;
	bool proxReadingReceived = false;
	bool proxUnsafe = false;
	bool proxInterrupted = false;
	uint64_t proxDeadlineMs = 0;
	uint64_t moveEndMs = 0;

	BaseBoard* baseboard = nullptr;
	Com* com = nullptr;
};

#endif //BUTTERBOT_FIRMWARE_MOVEMENTCOMMANDROUTINE_H
