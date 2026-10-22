#ifndef BUTTERBOT_FIRMWARE_PHRASES_H
#define BUTTERBOT_FIRMWARE_PHRASES_H

#include <array>
#include <span>
#include <string>
#include <vector>
#include "Log/Log.h"

DEFINE_LOG(Phrases)

enum class Phrase : uint16_t {
    None,
    Fact,
    LEDModuleInsert,
    LEDModuleRemove,
    PerfModuleInsert,
    PerfModuleRemove,
    PIRModuleInsert,
    PIRModuleRemove,
    GasModuleInsert,
    GasModuleRemove,
    TempHumModuleInsert,
    TempHumModuleRemove,
    IRModuleInsert,
    IRModuleRemove,
    IRModuleMissing,
    GasOver,
    GasUnder,
    BatteryLow,
    BatteryCritical,
    BatteryCharging,
    BatteryChargingFull,
    IntruderYes,
    IntruderNo,
    Joke,
    PassTheButter,
    Profanity,
    YouPassButter,
    Fall,
    UpsideDown,
    PickedUp,
    Shake,
	LEDModuleMissingPhrases,
	LEDTurnONPhrases,
	LEDAlreadyTurnONPhrases,
	LEDTurnOFFPhrases,
	LEDAlreadyTurnOFFPhrases,
	LEDStrobePhrases,
	LEDBreathePhrases,
	LEDFasterPhrases,
	LEDAlreadyFasterPhrases,
	LEDSlowerPhrases,
	LEDAlreadySlowerPhrases,
	Ramble,
	PokeLvl1,
	PokeLvl2,
	PokeLvl3,
	EightBall,
	EightBallListening,
	EightBallNoQuestion,
    TempHumModuleMissing,
    TempHumReading,
    TempHumScaleCelsius,
    TempHumScaleFahrenheit,
    TempHumScaleKelvin,
    DiceRollAskCount,
    DiceRollAskType,
    DiceRollCountTimeout,
    DiceRollCountInvalid,
    DiceRollTypeTimeout,
    DiceRollTypeInvalid,
	GasModuleMissing,
	AirQualityOK,
	AirQualityBad,
	TurningOff,
	PIRModuleMissing,
	IntruderDetectionOn,
	IntruderDetectionOff,
	PhoneNotConnected,
	PhoneConnected,
	PhoneDisconnected,
	PhoneNoNotifs,
	PhoneNotifCount,
	PhoneAskRead,
	PhoneAskContinue,
	PhoneAllRead,
	PhonePlaying,
	PhonePaused,
	PhoneNotPlaying,
	PhoneNextTrack,
	PhonePrevTrack,
	PhonePlayMusic,
	PhoneStopMusic,
	CurrentTimeShow,
	CurrentTimeNotConfigured,
	WhatsThisNone,
	WhatsThisOne,
	WhatsThisTwo,
	ObserveNone,
	ObserveBackpack,
	ObserveBottle,
	ObserveController,
	ObserveKeyboard,
	ObserveLamp,
	ObserveLaptop,
	ObserveMug,
	ObserveNotebook,
	ObservePhone,
	ObservePlant,
	ObserveLaptopMug,
	ObserveLaptopPhone,
	ObserveKeyboardMug,
	ObserveNotebookPhone,
	ObserveBackpackLaptop,
	ObservePlantLaptop,
	ObservePlantMug,
	ObserveControllerLaptop,
	ObserveControllerPhone,
	ObserveBackpackBottle,
	FaceScanStart,
	FaceNotDetected,
	FaceOwnerRecognized,
	FaceStrangerRecognized,
	FaceAlreadyOwner,
	FaceRegistered,
	FaceForgotten,
	FaceNoOwner,
	Startup,
	ListeningStart,
	ListeningTimeout,
    IRTrainFull,
    IRTrainScanning,
    IRTrainScanTimeout,
    IRTrainInvalid,
    IRTrainScanSuccess,
    IRTrainAskPhrase,
    IRTrainPhraseTimeout,
    IRTrainTooSimilar,
    IRTrainSaved,
    IRActionReserved,
    IRListEmpty,
    IRListPrefix,
    IRForgetAskCommand,
    IRForgetTimeout,
    IRForgetUnknown,
    IRForgetSuccess,
    IRActionDone,
    IRForgetAllEmpty,
    IRForgetAllSuccess,
	GasModuleFirstInsert,
	GasCalibrationFinished,
	UnknownModuleInsert,
	UnknownModuleRemove,
    VoiceForward,
    VoiceBackward,
    VoiceTurn180,
    VoiceTurnLeft,
    VoiceTurnRight,
    DanceStart,
    DanceInterlude,
    DanceStopped,
    DanceInterrupted,
    PersonOwnerGreeting,
    PersonStrangerGreeting,
    SummonStart,
    SummonFound,
    SummonNotFound,
    SummonCharging,
    CannotMove,
    CannotPerformPlugged,
    RoutineInterrupted,
    CameraFailure,
    MotorBoardFailure,
    ShuttingDown,
	ListenAbort,
	SettingsFrontSensorOff,
	SettingsFrontSensorOn,
	SettingsBottomSensorOff,
	SettingsBottomSensorReduced,
	SettingsBottomSensorOn,
	SettingsActivityOff,
	SettingsActivityChill,
	SettingsActivityActive,
	SettingsActivityHyper,
	SettingsAskActivity,
    // Add phrases here
    COUNT
};

class Phrases {
    friend class PhraseArrays;

public:
    // Reserves the get() selection scratch buffer once. Call during startup so no allocation happens on first phrase request.
    // Safe (but not required) to skip - get() falls back to allocating lazily on first use.
    static void preallocate();

    static std::string map(Phrase phrase, int16_t index);

    // Like map(), but returns the on-screen string for the phrase. Falls back to the pronounced string when 'shown' is empty.
    static std::string mapShown(Phrase phrase, int16_t index);

    static int16_t get(Phrase phrase);

private:
    struct PhraseOutput {
        const char* const pronounced; // The string the robot will pronounce (say out loud)
        float probability = 0.0f; // If <= 0.0f, it will be considered as uniform distribution with all others with probability <= 0.0f
        bool rare = false; // If true, first response for a phrase will not be this one unless its the only one
        const char* const shown = ""; // The string shown on the controller's screen. If empty, it is the same as 'pronounced'
    };

    static std::array<bool, static_cast<size_t>(Phrase::COUNT)> OutputTracking;

    // Tracks which outputs (by index into the output span) have already been returned for each phrase,
    // so the same output is not repeated until every output for that phrase has been returned at least once.
    static std::array<std::vector<bool>, static_cast<size_t>(Phrase::COUNT)> ReturnedOutputs;
};

#endif //BUTTERBOT_FIRMWARE_PHRASES_H
