#ifndef BUTTERBOT_COMMON_ACTIONDATA_H
#define BUTTERBOT_COMMON_ACTIONDATA_H

#include <ctime>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "Phrases.h"
#include "Serialization.h"
#include "CtrlData.h"

static constexpr const char* ControllerIP = "11.0.0.1";
static constexpr const char* ButterbotIP = "11.0.0.2";
static constexpr uint16_t UDPPort = 6001;

struct BB {
	enum class State {
		Idle, Listen, Scenario, RC
	} state;

	union Action {
		enum class Idle {
			None, Wander, Ramble, Person, Poke, Summon, Fall, UpsideDown, PickUp, Shake, ModuleChange, Gas, Intruder,
			BatteryLow, Charging, ChargingFull, PhoneNotif, PhoneConnect, Observe, GasConfigureStart, GasConfigureEnd,
			BatteryLevel, ShutUp, CantMove, SettingsChange, SettingsSync
		} idle;

		enum class Scenario {
			ReadNotifs, EightBall, Fact, Joke, PassTheButter, Profanity, YouPassButter, DiceRoll, CurrentTime,
			VoiceControl, Dance,
			FaceDetect, FaceDetectForget,
			IR_train, IR_forget, IR_forgetAll, IR_list, IR_action,
			LEDModule, LEDTurnOn, LEDTurnOff, LEDStrobe, LEDBreathe, LEDFaster, LEDSlower,
			GasModule,
			TempHumModule, TempHumScaleCelsius, TempHumScaleFahrenheit, TempHumScaleKelvin,
			IntruderDetectionOn, IntruderDetectionOff,
			Shutdown,
			PhoneNotConnected,
			PhoneListNotifs,
			PhoneWhatsPlaying,
			PhoneNextSong,
			PhonePrevSong,
			PhonePlayMusic,
			PhoneStopMusic,
			WhatsThis,
			SettingsActivity,
		} scenario;

		enum class Listen {
		} listen;

		enum class RC {
			FeedStatus
		} remoteControl;
	} action;

	size_t dataSize;
};

struct BBData {}; // Empty struct. All action-related data structs must inherit this

struct ListenData : BBData {
	enum class Phase : uint8_t {
		Listening, NotUnderstand, Abort
	} phase;
};

struct RambleData : BBData {
	uint8_t id;
};

struct WanderData : BBData {};

struct PersonData : BBData {
	uint8_t id;
	bool greetKnown;
};

struct FactData : BBData {
	uint8_t id;
};

struct JokeData : BBData {
	uint8_t id;
};

struct PassTheButterData : BBData {
	uint8_t id;
};

struct ProfanityData : BBData {
	uint8_t id;
};

struct YouPassButterData : BBData {
	uint8_t id;
};

struct PokeData : BBData {
	enum class Level : uint8_t {
		One, Two, Three, COUNT
	} level;

	uint8_t id;
};

struct ShutUpData : BBData {
	bool muted;
};

struct SummonData : BBData {
	enum class Phase : uint8_t {
		Start, Found, NotFound, Charging, Unsafe, Interrupted
	} phase;

	uint8_t id;
};

struct FallData : BBData {};

struct UpsideDownData : BBData {};

struct PickUpData : BBData {};

struct ShakeData : BBData {};

struct EightBallData : BBData {
	enum class Phase : uint8_t {
		Listening, Thinking, Response
	} phase;

	uint8_t responseId;
};

struct IR_trainData : BBData {
	enum class State : uint8_t {
		MaxCapacity, Scanning, ScanDone, Listening, SaveDone
	} state;

	uint8_t id;
	bool missing;
};

struct IR_forgetData : BBData {
	enum class State : uint8_t {
		Listening, NotRecognised, Done, Empty
	} state;
};

struct IR_forgetAllData : BBData {
	enum class State : uint8_t {
		Done, Empty
	} state;
};

struct IR_listData : BBData {
	enum class State : uint8_t {
		NoActions, ListingActions
	} state;
};

struct IR_actionData : BBData {
    uint8_t commandIndex;
};

struct GasData : BBData {
	uint8_t id;
	bool missing;
	bool ok;
};

struct IntruderData : BBData {
	uint8_t id;
	bool detected;
};

enum class ModuleType : uint8_t {
	Unknown = 0,
	TempHumidity,
	LED,
	Motion,
	CO2,
	IR,
	PerfBoard,
	COUNT
};

struct ModuleData : BBData {
	uint8_t id;
	ModuleType type;
	bool inserted;
};

struct BatteryData : BBData {
	uint8_t id;

	enum class State : uint8_t {
		Critical, Low, Charging, ChargingFull
	} state;
};

// Mirrors firmware ChargingState (BaseBoard.h). Order must match.
enum class ChargeStatus : uint8_t {
	Unplugged, Charging, Full
};

struct BatteryLevelData : BBData {
	uint8_t level;        // Battery::Level: 0 = Critical .. 6 = Full
	ChargeStatus charge;
};

struct LEDModuleData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDTurnOnData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDTurnOffData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDStrobeData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDBreatheData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDFasterData : BBData {
	uint8_t id;
	bool missing;
};

struct LEDSlowerData : BBData {
	uint8_t id;
	bool missing;
};

struct DiceRollData : BBData {
	enum class Phase : uint8_t {
		RollCount, DiceKind, RollAnim, Result
	} phase;

	enum class DiceType : uint8_t {
		D4, D6, D8, D10, D12, D20, D100, None
	} diceType;

	uint8_t diceCount;
	uint16_t diceResult;
};

struct CurrentTimeData : BBData {
	enum class Status : uint8_t {
		NoTime, ShowTime
	} status;

	tm currentTime;
};

struct VoiceControlData : BBData {
	enum class Direction : uint8_t {
		Forward, Backward, Left, Right, Rotate
	} direction;
};

struct DanceData : BBData {};

enum class TempHumScale : uint8_t {
	Celsius,
	Fahrenheit,
	Kelvin
};

struct TempHumModuleData : BBData {
	uint8_t id;
	bool missing;
	int16_t temperature;
	uint8_t humidity;
	TempHumScale scale;
};

struct TempHumScaleData : BBData {
	uint8_t id;
	TempHumScale scale;
};

struct ShutdownData : BBData {
	uint8_t id;

	enum class ShutdownReason : uint8_t {
		Command,
		Battery
	} reason;
};

struct PIRModuleData : BBData {
	uint8_t id;
	bool missing;
};

struct PhoneConnData : BBData {
	uint8_t id;
	bool connected;
};

struct PhoneNotConnectedData : BBData {
	uint8_t id;
};

struct PhoneListNotifsData : BBData {
	enum class Phase : uint8_t {
		NoNotifs, Count, AskRead, Reading, AskContinue, Done
	} phase;
	uint8_t id;
	uint8_t count;
	uint8_t remaining;
};

struct WhatsPlayingData : BBData {
	static constexpr uint8_t MaxField = 64;

	uint8_t id;
	std::string title;
	std::string artist;
	std::string album;

	std::vector<uint8_t> serialize() const{
		std::vector<uint8_t> buf;
		ComSerial::writeU8(buf, id);
		ComSerial::writeString(buf, title, MaxField);
		ComSerial::writeString(buf, artist, MaxField);
		ComSerial::writeString(buf, album, MaxField);
		return buf;
	}

	static WhatsPlayingData deserialize(const uint8_t* data, size_t size){
		WhatsPlayingData d;
		const uint8_t* cur = data;
		const uint8_t* end = data + size;
		d.id = ComSerial::readU8(cur, end);
		d.title = ComSerial::readString(cur, end);
		d.artist = ComSerial::readString(cur, end);
		d.album = ComSerial::readString(cur, end);
		return d;
	}
};

struct MediaControlData : BBData {
	uint8_t id;
};

struct FeedStatusData : BBData {
	bool status;
};

struct FaceDetectData : BBData {
	enum class State : uint8_t {
		Scanning, Recognized, NotRecognized, NoFaceFound, AlreadySaved, NewOwner
	} state;
};

struct FaceForgetData : BBData {
	enum class State : uint8_t {
		NoDataExist, Done
	} state;
};

struct WhatsThisData : BBData {
	Phrase phrase;
	uint8_t id;
	uint8_t count;
	uint8_t class1;
	uint8_t class2;
};

struct ObserveData : BBData {
	Phrase phrase;
	uint8_t id;
	uint8_t count;
	uint8_t class1;
	uint8_t class2;
};

struct GasConfigureStartData : BBData {
	uint8_t id;
};

struct GasConfigureEndData : BBData {
	uint8_t id;
};

struct CantMoveData : BBData {
	enum class Reason : uint8_t {
		Charging, Unstable
	} reason;
};

struct SettingsChangeData : BBData {
	Phrase phrase;
	uint8_t id;
};

struct SettingsSyncData : BBData {
	FrontSensorMode frontSensor;
	BottomSensorMode bottomSensor;
	ActivityMode activityMode;
};

#endif //BUTTERBOT_COMMON_ACTIONDATA_H
