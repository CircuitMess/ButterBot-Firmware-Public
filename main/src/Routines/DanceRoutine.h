#ifndef BUTTERBOT_FIRMWARE_DANCEROUTINE_H
#define BUTTERBOT_FIRMWARE_DANCEROUTINE_H

#include <atomic>
#include <cstdint>
#include <string>
#include <Phrases.h>
#include "Routine.h"
#include "Audio/AudioFrontend.h"
#include "Audio/SpeechAudioGen.h"
#include "Audio/SpeechGen.h"
#include "Services/BaseBoard.h"
#include "Services/MotionService.h"
#include "Battery/Battery.h"
#include "Services/Com.h"
#include "Services/Modules/ModuleService.h"
#include "Services/ISRButtonInput.h"
#include <Services/Audio/Audio.h>
#include "CtrlData.h"

class DanceRoutine : public Routine {
public:
	using Routine::Routine;

public:
	virtual ~DanceRoutine() override;
	TickingState tick(float deltaTime) override;

private:
	void onModuleEvent(uint8_t bus, Modules::Type type, ModuleService::Action action);
	void onCommand(Ctrl::Command command);
	void onButtonEvent(int button, ISRButtonInput::Action action);
	void onProximityReading(ProxState front, ProxState bottom);
	void onProximityChange(ProxSensor sensor, bool state);
	void onMotion(MotionType type);
	void onStopPhrase(bool recognized, int index, const std::string& transcript, float confidence);

	enum class Phase : uint8_t { Init, ProxCheck, Dancing, Cleanup };

	enum class StopReason : uint8_t { Natural, Voice, Interrupted };

	struct DanceMove {
		int8_t motorLeft;
		int8_t motorRight;
		uint32_t durationMs;
	};

	static const DanceMove DanceMoves[];
	static constexpr uint8_t DanceMoveCount = 9;

	static constexpr uint32_t DanceMinMs = 8000;
	static constexpr uint32_t DanceMaxMs = 25000;
	static constexpr uint32_t InterludeCheckIntervalMinMs = 3000;
	static constexpr uint32_t InterludeCheckIntervalMaxMs = 5000;
	static constexpr uint8_t InterludeChancePercent = 35; // Probability BB will say something during dancing
	static constexpr uint32_t ProxReadingTimeoutMs = 500;

	Phase phase = Phase::Init;
	StopReason stopReason = StopReason::Natural;
	bool started = false;
	bool finished = false;
	uint64_t proxDeadlineMs = 0;

	uint64_t danceEndMs = 0;
	uint64_t moveEndMs = 0;
	uint64_t nextInterludeCheckMs = 0;
	uint8_t currentMoveIndex = 0;
	bool waitingForCounterpart = false;
	uint8_t counterpartIndex = 0;
	uint8_t lastPairIndex = 255;

	std::atomic<bool> interrupted{ false };
	std::atomic<bool> stopVoiceDetected{ false };
	std::atomic<bool> listeningForStop{ false };
	std::atomic<bool> proxReadingReceived{ false };
	std::atomic<bool> proxUnsafe{ false };

	BaseBoard* baseBoard = nullptr;
	AudioFrontend* audioFrontend = nullptr;
	MotionService* motionService = nullptr;
	Battery* battery = nullptr;
	ModuleService* moduleService = nullptr;
	Com* com = nullptr;
	ISRButtonInput* buttonInput = nullptr;

	void bindEvents();
	void unbindEvents();
	void pickRandomMove();
	static void playPhrase(Phrase phrase, Audio* audio);
	void startListeningForStop();
};

#endif //BUTTERBOT_FIRMWARE_DANCEROUTINE_H
