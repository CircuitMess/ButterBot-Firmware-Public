#ifndef BUTTERBOT_FIRMWARE_SETTINGSACTIVITYROUTINE_H
#define BUTTERBOT_FIRMWARE_SETTINGSACTIVITYROUTINE_H

#include <cstdint>
#include <CtrlData.h>
#include "PhraseListeningRoutine.h"

class SettingsActivityRoutine : public PhraseListeningRoutine {
public:
	explicit SettingsActivityRoutine(BBStateMachine* sm) : PhraseListeningRoutine(sm) {}

	TickingState tick(float deltaTime) override;

protected:
	bool levelKnown = false;
	ActivityMode knownLevel = ActivityMode::Off;

private:
	enum class Phase : uint8_t { Init, WaitLevel };
	Phase phase = Phase::Init;
};

class SettingsActivityOffRoutine : public SettingsActivityRoutine {
public:
	explicit SettingsActivityOffRoutine(BBStateMachine* sm);
};

class SettingsActivityChillRoutine : public SettingsActivityRoutine {
public:
	explicit SettingsActivityChillRoutine(BBStateMachine* sm);
};

class SettingsActivityActiveRoutine : public SettingsActivityRoutine {
public:
	explicit SettingsActivityActiveRoutine(BBStateMachine* sm);
};

class SettingsActivityHyperRoutine : public SettingsActivityRoutine {
public:
	explicit SettingsActivityHyperRoutine(BBStateMachine* sm);
};

#endif //BUTTERBOT_FIRMWARE_SETTINGSACTIVITYROUTINE_H