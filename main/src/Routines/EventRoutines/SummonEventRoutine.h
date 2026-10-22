#ifndef BUTTERBOT_FIRMWARE_SUMMONEVENTROUTINE_H
#define BUTTERBOT_FIRMWARE_SUMMONEVENTROUTINE_H

#include <cstdint>
#include <glm.hpp>
#include <Phrases.h>
#include <BBData.h>
#include "Routines/EventRoutine.h"
#include "Services/BaseBoard.h"
#include "Services/MotionService.h"

class FaceDet;

/**
 * Summon routine, triggered by the controller's Summon command (routed through EventBag). Behaves
 * like PersonRoutine: enables face detection and, while no face is visible, rotates 45 degrees in a
 * fixed random direction up to eight times (a full 360). Face detection is paused while rotating, so
 * faces are only picked up during the stationary scan windows. On detecting a face it centers on it,
 * drives towards it briefly, then announces success. Any face - stored owner or not - is treated the
 * same. Announces the search on start, and announces failure if no face is ever seen.
 *
 * Preconditions checked at start (skipped otherwise): not charging, and upright on the ground.
 * MotionService is enabled by IdleState, which this routine relies on for the upright/ground check.
 */
class SummonEventRoutine : public EventRoutine {
public:
	using EventRoutine::EventRoutine;

	~SummonEventRoutine() override;
	TickingState tick(float deltaTime) override;

private:
	void onDetect(int faceCount, bool known, glm::vec2 pos, glm::vec2 size);
	void onProximityReading(ProxState front, ProxState bottom);
	void onProximityChange(ProxSensor sensor, bool state);
	void onMotion(MotionType type);
	void beginApproach();
	void speak(Phrase phrase, SummonData::Phase dataPhase);

	enum class Phase : uint8_t { Init, ProxCheck, Scanning, Rotating, Centering, Driving };

	static constexpr uint32_t ScanWindowMs = 700; // ~4 detection frames at ~150ms per frame
	static constexpr uint32_t Rotate45Ms = 1000; // VoiceTurn180 is 4000ms at full speed, so 45 is ~1000ms
	static constexpr uint8_t MaxRotations = 8;   // 8 x 45 = full 360
	static constexpr int8_t RotateSpeed = 100;
	static constexpr int8_t CenterSpeed = 60;
	static constexpr float CenterDeadband = 0.15f;
	static constexpr uint32_t CenterTimeoutMs = 4000;
	static constexpr int8_t DriveSpeed = 100;
	static constexpr uint32_t DriveMinMs = 1000;
	static constexpr uint32_t DriveMaxMs = 2000;
	static constexpr uint32_t ProxReadingTimeoutMs = 500;

	Phase phase = Phase::Init;
	bool faceDetActive = false;
	bool proxReadingReceived = false;
	bool proxUnsafe = false;
	bool proxInterrupted = false;
	uint64_t proxDeadlineMs = 0;

	// Updated by onDetect on the StateMachine task, read by tick on the same task - a plain
	// member is sufficient (see PersonRoutine).
	bool detected = false;
	float faceCenterX = 0.5f;

	int8_t scanMotorLeft = 0;
	int8_t scanMotorRight = 0;
	uint8_t rotationCount = 0;

	uint64_t scanDeadlineMs = 0;
	uint64_t rotateDeadlineMs = 0;
	uint64_t centerDeadlineMs = 0;
	uint64_t driveEndMs = 0;

	BaseBoard* baseBoard = nullptr;
	FaceDet* faceDet = nullptr;
};

#endif //BUTTERBOT_FIRMWARE_SUMMONEVENTROUTINE_H
