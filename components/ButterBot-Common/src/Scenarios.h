#ifndef BUTTERBOTCTRL_FIRMWARE_SCENARIOS_H
#define BUTTERBOTCTRL_FIRMWARE_SCENARIOS_H

#include <array>
#include <cstdint>
#include "BBData.h"

struct ScenarioData {
	uint8_t raw = 0;

	constexpr bool operator==(const ScenarioData& other) const{
		return raw == other.raw;
	}
};

struct DiceScenarioData : ScenarioData {
	enum class DiceType : uint8_t {
		None, D4, D6, D8, D10, D12, D20, D100
	};

	constexpr DiceScenarioData(DiceType diceType) : ScenarioData{ (uint8_t) diceType }{}

	constexpr DiceType diceType() const{
		return (DiceType) raw;
	}
};

struct VoiceScenarioData : ScenarioData {
	enum class Direction : uint8_t {
		Forward, Backward, Left, Right, Rotate
	};

	constexpr VoiceScenarioData(Direction direction) : ScenarioData{ (uint8_t) direction }{}

	constexpr Direction direction() const{
		return (Direction) raw;
	}
};

struct FaceScenarioData : ScenarioData {
	enum class Phase : uint8_t {
		Detect, Remember, Forget
	};

	constexpr FaceScenarioData(Phase phase) : ScenarioData{ (uint8_t) phase }{}

	constexpr Phase phase() const{
		return (Phase) raw;
	}
};

struct ActivityScenarioData : ScenarioData {
	enum class Level : uint8_t {
		None, Off, Chill, Active, Hyper
	};

	constexpr ActivityScenarioData(Level level) : ScenarioData{ (uint8_t) level }{}

	constexpr Level level() const{
		return (Level) raw;
	}
};

static constexpr float DefaultFuzzyThreshold = 0.2f;

struct ScenarioActivation {
	BB::Action::Scenario scenario;
	ScenarioData scenarioData = {};

	const char* string; //Phrase string (graphemes)
	const char* phonemes; //Phrase phonemes, generated using multinet_g2p.py[esp-sr/tool/multinet_g2p.py]
	const char* fuzzyCore = nullptr;
	float fuzzyThreshold = DefaultFuzzyThreshold;
	const void* action = nullptr;
};

constexpr std::array activations = std::to_array<ScenarioActivation>(
	{
		{ BB::Action::Scenario::Fact, {}, "tell me a fact", "TfL Mm c FaKT", "FaKT", 0.3f },
		{ BB::Action::Scenario::Fact, {}, "give me a fact", "GgV Mm c FaKT", "FaKT", 0.3f },
		{ BB::Action::Scenario::Fact, {}, "i want a fact", "i WnNT c FaKT", "FaKT", 0.3f },
		{ BB::Action::Scenario::Fact, {}, "do you know a fact", "Do Yo Nb c FaKT", "FaKT", 0.3f },
		{ BB::Action::Scenario::Fact, {}, "tell me something interesting", "TfL Mm ScMvgl gNTRcSTgl", "gNTRcSTgl" },
		{ BB::Action::Scenario::Fact, {}, "hit me with a fact", "hgT Mm Wgj c FaKT", "FaKT" },
		{ BB::Action::Scenario::Fact, {}, "drop some knowledge", "DRnP ScM NnLcq", "NnLcq" },
		{ BB::Action::Scenario::Fact, {}, "blow my mind", "BLb Mi MiND", "BLb MiND" },
		{ BB::Action::Scenario::Fact, {}, "teach me something new", "Tmp Mm ScMvgl No", "Tmp No" },
		{ BB::Action::Scenario::Joke, {}, "tell me a joke", "TfL Mm c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "make a joke", "MdK c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "do you know a joke", "Do Yo Nb c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "give me a joke", "GgV Mm c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "i want a joke", "i WnNT c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "say something funny", "Sd ScMvgl FcNm", "FcNm" },
		{ BB::Action::Scenario::Joke, {}, "make me laugh", "MdK Mm LaF", "LaF" },
		{ BB::Action::Scenario::Joke, {}, "crack a joke", "KRaK c qbK", "qbK" },
		{ BB::Action::Scenario::Joke, {}, "amuse me", "cMYoZ Mm", "cMYoZ" },
		{ BB::Action::Scenario::Joke, {}, "give me some humor", "GgV Mm ScM hYoMk", "hYoMk" },
		{ BB::Action::Scenario::PassTheButter, {}, "pass the butter", "PaS jc BcTk", "BcTk", 0.27f },
		{ BB::Action::Scenario::PassTheButter, {}, "give me the butter", "GgV Mm jc BcTk", "BcTk", 0.24f },
		{ BB::Action::Scenario::PassTheButter, {}, "hand me the butter", "haND Mm jc BcTk", "BcTk", 0.27f },
		{ BB::Action::Scenario::PassTheButter, {}, "bring the butter", "BRgl jc BcTk", "BcTk", 0.27f },
		{ BB::Action::Scenario::PassTheButter, {}, "i need the butter", "i NmD jc BcTk", "BcTk", 0.27f },
		{ BB::Action::Scenario::PassTheButter, {}, "butter please", "BcTk PLmZ", "BcTk", 0.27f },
		{ BB::Action::Scenario::PassTheButter, {}, "can you pass the butter", "KaN Yo PaS jc BcTk", "BcTk", 0.27f },
		{ BB::Action::Scenario::YouPassButter, {}, "you pass butter", "Yo PaS BcTk", "PaS BcTk", 0.25f },
		{ BB::Action::Scenario::Profanity, {}, "you are stupid", "Yo nR SToPcD", "SToPcD" },
		{ BB::Action::Scenario::Profanity, {}, "you are useless", "Yo nR YoSLcS", "YoSLcS" },
		{ BB::Action::Scenario::Profanity, {}, "you are dumb", "Yo nR DcM", "DcM" },
		{ BB::Action::Scenario::Profanity, {}, "you are broken", "Yo nR BRbKcN", "BRbKcN", 0.25f },
		{ BB::Action::Scenario::Profanity, {}, "you do not work", "Yo Do NnT WkK", "WkK" },
		{ BB::Action::Scenario::Profanity, {}, "you are worthless", "Yo nR WkvLcS", "WkvLcS", 0.25f },
		{ BB::Action::Scenario::Profanity, {}, "you are bad at this", "Yo nR BaD aT jgS", "BaD" },
		{ BB::Action::Scenario::Profanity, {}, "you are terrible", "Yo nR TfRcBcL", "TfRcBcL" },
		{ BB::Action::Scenario::Profanity, {}, "what is wrong with you", "WcT gZ Rel Wgj Yo", "Rel" },
		{ BB::Action::Scenario::Profanity, {}, "you idiot", "Yo gDmcT", "gDmcT", 0.3f },
		{ BB::Action::Scenario::Profanity, {}, "moron", "MeRnN", "MeRnN" },
		{ BB::Action::Scenario::Profanity, {}, "you failure", "Yo FdLYk", "FdLYk" },
		{ BB::Action::Scenario::Profanity, {}, "waste of time", "WdST cV TiM", "WdST", 0.27f },
		{ BB::Action::Scenario::Profanity, {}, "you are disappointing", "Yo nR DgScPuNTgl", "DgScPuNTgl" },
		{ BB::Action::Scenario::Profanity, {}, "screw you", "SKRo Yo", "SKRo" },
		{ BB::Action::Scenario::Profanity, {}, "you suck", "Yo ScK", "ScK", 0.3f },
		{ BB::Action::Scenario::Profanity, {}, "you are a piece of junk", "Yo nR c PmS cV qclK", "PmS cV qclK" },
		{ BB::Action::Scenario::Profanity, {}, "you are garbage", "Yo nR GnRBgq", "GnRBgq" },
		{ BB::Action::Scenario::Profanity, {}, "you are trash", "Yo nR TRas", "TRas" },
		{ BB::Action::Scenario::Profanity, {}, "absolute failure", "aBScLoT FdLYk", "FdLYk" },
		{ BB::Action::Scenario::Profanity, {}, "complete waste", "KcMPLmT WdST", "WdST" },
		{ BB::Action::Scenario::Profanity, {}, "worthless machine", "WkvLcS McsmN", "WkvLcS" },
		{ BB::Action::Scenario::Profanity, {}, "stupid robot", "SToPcD RbBnT", "SToPcD" },
		{ BB::Action::Scenario::Profanity, {}, "stupid machine", "SToPcD McsmN", "SToPcD" },
		{ BB::Action::Scenario::LEDTurnOn, {}, "turn on the lights", "TkN nN jc LiTS", "nN LiTS", 0.25f },
		{ BB::Action::Scenario::LEDTurnOn, {}, "switch on the lights", "SWgp nN jc LiTS", "nN LiTS", 0.25f },
		{ BB::Action::Scenario::LEDTurnOn, {}, "power on the lights", "Ptk nN jc LiTS", "nN LiTS", 0.25f },
		{ BB::Action::Scenario::LEDTurnOn, {}, "enable the lights", "fNdBcL jc LiTS", "fNdBcL LiTS" },
		{ BB::Action::Scenario::LEDTurnOn, {}, "lights on", "LiTS nN", "LiTS nN" },
		{ BB::Action::Scenario::LEDTurnOff, {}, "turn off the lights", "TkN eF jc LiTS", "eF LiTS" },
		{ BB::Action::Scenario::LEDTurnOff, {}, "switch off the lights", "SWgp eF jc LiTS", "eF LiTS" },
		{ BB::Action::Scenario::LEDTurnOff, {}, "power off the lights", "Ptk eF jc LiTS", "eF LiTS" },
		{ BB::Action::Scenario::LEDTurnOff, {}, "disable the lights", "DgSdBcL jc LiTS", "DgSdBcL LiTS" },
		{ BB::Action::Scenario::LEDTurnOff, {}, "lights off", "LiTS eF", "LiTS eF" },
		{ BB::Action::Scenario::LEDStrobe, {}, "light strobe", "LiT STRbB", "STRbB", 0.3f },
		{ BB::Action::Scenario::LEDStrobe, {}, "Turn on strobe", "TkN nN STRbB", "STRbB", 0.3f },
		{ BB::Action::Scenario::LEDStrobe, {}, "Enable strobe", "fNdBcL STRbB", "STRbB", 0.3f },
		{ BB::Action::Scenario::LEDStrobe, {}, "Start strobe", "STnRT STRbB", "STRbB", 0.3f },
		{ BB::Action::Scenario::LEDStrobe, {}, "light flash", "LiT FLas", "FLas", 0.25f },
		{ BB::Action::Scenario::LEDBreathe, {}, "light breathe", "LiT BRmj", "BRmj", 0.25f },
		{ BB::Action::Scenario::LEDBreathe, {}, "Turn on breathing", "TkN nN BRmjgl", "BRmjgl", 0.21 },
		{ BB::Action::Scenario::LEDBreathe, {}, "Enable breathing", "fNdBcL BRmjgl", "BRmjgl" },
		{ BB::Action::Scenario::LEDBreathe, {}, "Start breathing", "STnRT BRmjgl", "BRmjgl" },
		{ BB::Action::Scenario::LEDBreathe, {}, "lights pulse", "LiTS PcLS", "PcLS" },
		{ BB::Action::Scenario::LEDFaster, {}, "lights faster", "LiTS FaSTk", "FaSTk", 0.3f },
		{ BB::Action::Scenario::LEDSlower, {}, "lights slower", "LiTS SLbk", "SLbk" },
		{ BB::Action::Scenario::TempHumModule, {}, "temperature report", "TfMPRcpk RgPeRT", "TfMPRcpk", 0.25f },
		{ BB::Action::Scenario::TempHumModule, {}, "humidity report", "hYoMgDcTm RgPeRT", "hYoMgDcTm", 0.3f },
		{ BB::Action::Scenario::TempHumModule, {}, "climate report", "KLiMcT RgPeRT", "KLiMcT", 0.3f },
		{ BB::Action::Scenario::TempHumModule, {}, "show temperature and humidity", "sb TfMPRcpk cND hYoMgDcTm", "TfMPRcpk hYoMgDcTm", 0.3f },
		{ BB::Action::Scenario::TempHumModule, {}, "check temperature", "pfK TfMPRcpk", "TfMPRcpk", 0.25f },
		{ BB::Action::Scenario::TempHumModule, {}, "check humidity", "pfK hYoMgDcTm", "hYoMgDcTm", 0.3f },
		{ BB::Action::Scenario::TempHumModule, {}, "what is the temperature", "WcT gZ jc TfMPRcpk", "TfMPRcpk", 0.25f },
		{ BB::Action::Scenario::TempHumModule, {}, "what is the humidity", "WcT gZ jc hYoMgDcTm", "hYoMgDcTm", 0.3f },
		{ BB::Action::Scenario::TempHumModule, {}, "climate status", "KLiMcT STaTcS", "KLiMcT", 0.3f },
		{ BB::Action::Scenario::TempHumScaleCelsius, {}, "switch to celsius", "SWgp To SfLSmcS", "SfLSmcS", 0.3f },
		{ BB::Action::Scenario::TempHumScaleCelsius, {}, "set celsius", "SfT SfLSmcS", "SfLSmcS", 0.3f },
		{ BB::Action::Scenario::TempHumScaleCelsius, {}, "use celsius", "YoZ SfLSmcS", "SfLSmcS", 0.3f },
		{ BB::Action::Scenario::TempHumScaleCelsius, {}, "change to celsius", "pdNq To SfLSmcS", "SfLSmcS", 0.3f },
		{ BB::Action::Scenario::TempHumScaleCelsius, {}, "celsius mode", "SfLSmcS MbD", "SfLSmcS", 0.3f },
		{ BB::Action::Scenario::TempHumScaleFahrenheit, {}, "switch to fahrenheit", "SWgp To FfRcNhiT", "FfRcNhiT" },
		{ BB::Action::Scenario::TempHumScaleFahrenheit, {}, "set fahrenheit", "SfT FfRcNhiT", "FfRcNhiT" },
		{ BB::Action::Scenario::TempHumScaleFahrenheit, {}, "use fahrenheit", "YoZ FfRcNhiT", "FfRcNhiT" },
		{ BB::Action::Scenario::TempHumScaleFahrenheit, {}, "change to fahrenheit", "pdNq To FfRcNhiT", "FfRcNhiT" },
		{ BB::Action::Scenario::TempHumScaleFahrenheit, {}, "fahrenheit mode", "FfRcNhiT MbD", "FfRcNhiT" },
		{ BB::Action::Scenario::TempHumScaleKelvin, {}, "switch to kelvin", "SWgp To KfLVcN", "KfLVcN", 0.3f },
		{ BB::Action::Scenario::TempHumScaleKelvin, {}, "set kelvin", "SfT KfLVcN", "KfLVcN", 0.3f },
		{ BB::Action::Scenario::TempHumScaleKelvin, {}, "use kelvin", "YoZ KfLVcN", "KfLVcN", 0.3f },
		{ BB::Action::Scenario::TempHumScaleKelvin, {}, "change to kelvin", "pdNq To KfLVcN", "KfLVcN", 0.3f },
		{ BB::Action::Scenario::TempHumScaleKelvin, {}, "kelvin mode", "KfLVcN MbD", "KfLVcN", 0.3f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } /*DiceRollRoutine*/, "roll a dice", "RbL c DiS", "DiS", 0.1f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D4 } /*DiceRollD4Routine*/, "roll a d four", "RbL c Dm FeR", "FeR", 0.25f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D6 } /*DiceRollD6Routine*/, "roll a d six", "RbL c Dm SgKS", "SgKS", 0.3f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D8 } /*DiceRollD8Routine*/, "roll a d eight", "RbL c Dm dT", "dT", 0.1f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D10 } /*DiceRollD10Routine*/, "roll a d ten", "RbL c Dm TfN", "TfN" },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D12 } /*DiceRollD12Routine*/, "roll a d twelve", "RbL c Dm TWfLV", "TWfLV" },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D20 } /*DiceRollD20Routine*/, "roll a d twenty", "RbL c Dm TWfNTm", "TWfNTm" },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D20 } /*DiceRollD20Routine*/, "roll a d twenty", "RbL c Dm TWfNm", "TWfNm" }, // reduced "twenny"
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D100 } /*DiceRollD100Routine*/, "roll a d one hundred", "RbL c Dm WcN hcNDRcD", "WcN hcNDRcD" },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } /*DiceRollRoutine*/, "roll dice", "RbL DiS", "DiS", 0.27f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } /*DiceRollRoutine*/, "roll the dice", "RbL jc DiS", "DiS", 0.27f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } /*DiceRollRoutine*/, "i want to roll dice", "i WnNT To RbL DiS", "DiS", 0.13f },
		{ BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } /*DiceRollRoutine*/, "dice roll", "DiS RbL", "DiS", 0.21f },
		{ BB::Action::Scenario::GasModule, {}, "get air quality", "GfT fR KWnLcTm", "KWnLcTm" },
		{ BB::Action::Scenario::GasModule, {}, "check air quality", "pfK fR KWnLcTm", "KWnLcTm" },
		{ BB::Action::Scenario::GasModule, {}, "air quality status", "fR KWnLcTm STaTcS", "KWnLcTm" },
		{ BB::Action::Scenario::GasModule, {}, "what is the air quality", "WcT gZ jc fR KWnLcTm", "KWnLcTm" },
		{ BB::Action::Scenario::IntruderDetectionOn, {}, "enable intruder detection", "fNdBcL gNTRoDk DgTfKscN", "fNdBcL gNTRoDk", 0.35f },
		{ BB::Action::Scenario::IntruderDetectionOn, {}, "motion sensor on", "MbscN SfNSk nN", "MbscN SfNSk nN", 0.35f },
		{ BB::Action::Scenario::IntruderDetectionOn, {}, "turn on intruder detection", "TkN nN gNTRoDk DgTfKscN", "nN gNTRoDk", 0.35f },
		{ BB::Action::Scenario::IntruderDetectionOff, {}, "disable intruder detection", "DgSdBcL gNTRoDk DgTfKscN", "DgSdBcL gNTRoDk", 0.35f },
		{ BB::Action::Scenario::IntruderDetectionOff, {}, "motion sensor off", "MbscN SfNSk eF", "MbscN SfNSk eF", 0.35f },
		{ BB::Action::Scenario::IntruderDetectionOff, {}, "turn off intruder detection", "TkN eF gNTRoDk DgTfKscN", "eF gNTRoDk", 0.35f },
		{ BB::Action::Scenario::EightBall, {}, "i need advice", "i NmD cDViS", "cDViS", 0.3f },
		{ BB::Action::Scenario::EightBall, {}, "give me advice", "GgV Mm cDViS", "cDViS", 0.15f },
		{ BB::Action::Scenario::EightBall, {}, "any advice", "fNm cDViS", "cDViS" },
		{ BB::Action::Scenario::EightBall, {}, "help me decide", "hfLP Mm DgSiD", "DgSiD", 0.3f },
		{ BB::Action::Scenario::EightBall, {}, "what do you think", "WcT Do Yo vglK", "vglK" },
		{ BB::Action::Scenario::EightBall, {}, "i need guidance", "i NmD GiDcNS", "GiDcNS", 0.3f },
		{ BB::Action::Scenario::EightBall, {}, "can you help me", "KaN Yo hfLP Mm", "hfLP Mm" },
		{ BB::Action::Scenario::EightBall, {}, "help me out", "hfLP Mm tT", "hfLP Mm tT" },
		{ BB::Action::Scenario::EightBall, {}, "tell me what to do", "TfL Mm WcT To Do", "WcT To Do" },
		{ BB::Action::Scenario::EightBall, {}, "show me the way", "sb Mm jc Wd", "jc Wd" },
		{ BB::Action::Scenario::CurrentTime, {}, "what time is it", "WcT TiM gZ gT", "TiM", 0.15f },
		{ BB::Action::Scenario::CurrentTime, {}, "tell me the time", "TfL Mm jc TiM", "TiM" },
		{ BB::Action::Scenario::CurrentTime, {}, "current time", "KkcNT TiM", "TiM" },
		{ BB::Action::Scenario::CurrentTime, {}, "give me the time", "GgV Mm jc TiM", "TiM", 0.15f },
		{ BB::Action::Scenario::CurrentTime, {}, "time now", "TiM Nt", "TiM" },
		{ BB::Action::Scenario::CurrentTime, {}, "do you know the time", "Do Yo Nb jc TiM", "TiM" },
		{ BB::Action::Scenario::CurrentTime, {}, "what is the current time", "WcT gZ jc KkcNT TiM", "TiM" },
		{ BB::Action::Scenario::WhatsThis, {}, "what is this", "WcT gZ jgS", "jgS" },
		{ BB::Action::Scenario::WhatsThis, {}, "what's this", "WcTS jgS", "jgS" },
		{ BB::Action::Scenario::WhatsThis, {}, "what do you see", "WcT Do Yo Sm", "Sm" },
		{ BB::Action::Scenario::WhatsThis, {}, "what is in front of you", "WcT gZ gN FRcNT cV Yo", "FRcNT" },
		{ BB::Action::Scenario::WhatsThis, {}, "what's in front of you", "WcTS gN FRcNT cV Yo", "FRcNT" },
		{ BB::Action::Scenario::Shutdown, {}, "shut down", "scT DtN", "scT DtN" },
		{ BB::Action::Scenario::Shutdown, {}, "deactivate", "DmaKTgVdT", "DmaKTgVdT" },
		{ BB::Action::Scenario::Shutdown, {}, "shut yourself down", "scT YkSfLF DtN", "scT DtN" },
		{ BB::Action::Scenario::Shutdown, {}, "power down", "Ptk DtN", "Ptk DtN" },
		{ BB::Action::Scenario::Shutdown, {}, "turn yourself off", "TkN YkSfLF eF", "YkSfLF eF" },
		{ BB::Action::Scenario::Shutdown, {}, "battery off", "BaTkm eF", "BaTkm eF" },
		{ BB::Action::Scenario::IR_train, {}, "learn new infrared command", "LkN No gNFRkfD KcMaND", "LkN gNFRkfD" },
		{ BB::Action::Scenario::IR_train, {}, "train infrared command", "TRdN gNFRkfD KcMaND", "TRdN gNFRkfD", 0.25f },
		{ BB::Action::Scenario::IR_train, {}, "record infrared signal", "RgKeRD gNFRkfD SgGNcL", "RgKeRD gNFRkfD", 0.28f },
		{ BB::Action::Scenario::IR_train, {}, "add new infrared action", "aD No gNFRkfD aKscN", "aD gNFRkfD", 0.25f },
		{ BB::Action::Scenario::IR_list, {}, "list infrared commands", "LgST gNFRkfD KcMaNDS", "LgST gNFRkfD", 0.27f },
		{ BB::Action::Scenario::IR_list, {}, "what infrared commands do you know", "WcT gNFRkfD KcMaNDS Do Yo Nb", "gNFRkfD KcMaNDS", 0.3f },
		{ BB::Action::Scenario::IR_forget, {}, "forget infrared", "FkGfT gNFRkfD", "FkGfT gNFRkfD", 0.3f },
		{ BB::Action::Scenario::IR_forget, {}, "remove infrared", "RmMoV gNFRkfD", "RmMoV gNFRkfD" },
		{ BB::Action::Scenario::IR_forget, {}, "delete infrared", "DgLmT gNFRkfD", "DgLmT gNFRkfD" },
		{ BB::Action::Scenario::IR_forgetAll, {}, "forget all infrared", "FkGfT eL gNFRkfD", "FkGfT eL gNFRkfD", 0.25f },
		{ BB::Action::Scenario::IR_forgetAll, {}, "delete all infrared", "DgLmT eL gNFRkfD", "DgLmT eL gNFRkfD", 0.25f },
		{ BB::Action::Scenario::IR_forgetAll, {}, "clear all infrared", "KLgR eL gNFRkfD", "KLgR eL gNFRkfD" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "list notifications", "LgST NbTcFcKdscNZ", "NbTcFcKdscNZ" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "show notifications", "sb NbTcFcKdscNZ", "NbTcFcKdscNZ" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "what notifications do i have", "WcT NbTcFcKdscNZ Do i haV", "NbTcFcKdscNZ" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "read my notifications", "RmD Mi NbTcFcKdscNZ", "NbTcFcKdscNZ" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "check notifications", "pfK NbTcFcKdscNZ", "NbTcFcKdscNZ", 0.3f },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "do i have notifications", "Do i haV NbTcFcKdscNZ", "NbTcFcKdscNZ" },
		{ BB::Action::Scenario::PhoneListNotifs, {}, "tell me my notifications", "TfL Mm Mi NbTcFcKdscNZ", "NbTcFcKdscNZ", 0.23f },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "what is playing", "WcT gZ PLdgl", "PLdgl" },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "what song is playing", "WcT Sel gZ PLdgl", "PLdgl" },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "what is this song", "WcT gZ jgS Sel", "Sel" },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "current song", "KkcNT Sel", "Sel", 0.25f },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "what music is playing", "WcT MYoZgK gZ PLdgl", "MYoZgK PLdgl" },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "tell me what is playing", "TfL Mm WcT gZ PLdgl", "PLdgl" },
		{ BB::Action::Scenario::PhoneWhatsPlaying, {}, "what is playing right now", "WcT gZ PLdgl RiT Nt", "PLdgl" },
		{ BB::Action::Scenario::PhoneNextSong, {}, "next song", "NfKST Sel", "NfKST" },
		{ BB::Action::Scenario::PhoneNextSong, {}, "skip song", "SKgP Sel", "SKgP" },
		{ BB::Action::Scenario::PhoneNextSong, {}, "play next", "PLd NfKST", "NfKST", 0.3f },
		{ BB::Action::Scenario::PhoneNextSong, {}, "next track", "NfKST TRaK", "NfKST" },
		{ BB::Action::Scenario::PhonePrevSong, {}, "previous song", "PRmVmcS Sel", "PRmVmcS" },
		{ BB::Action::Scenario::PhonePrevSong, {}, "play previous", "PLd PRmVmcS", "PRmVmcS" },
		{ BB::Action::Scenario::PhonePrevSong, {}, "previous track", "PRmVmcS TRaK", "PRmVmcS" },
		{ BB::Action::Scenario::PhonePlayMusic, {}, "play music", "PLd MYoZgK", "PLd MYoZgK" },
		{ BB::Action::Scenario::PhonePlayMusic, {}, "start music", "STnRT MYoZgK", "STnRT MYoZgK" },
		{ BB::Action::Scenario::PhonePlayMusic, {}, "start playback", "STnRT PLdBaK", "STnRT PLdBaK" },
		{ BB::Action::Scenario::PhonePlayMusic, {}, "resume music", "RgZoM MYoZgK", "RgZoM MYoZgK" },
		{ BB::Action::Scenario::PhoneStopMusic, {}, "stop music", "STnP MYoZgK", "STnP MYoZgK" },
		{ BB::Action::Scenario::PhoneStopMusic, {}, "pause music", "PeZ MYoZgK", "PeZ MYoZgK" },
		{ BB::Action::Scenario::PhoneStopMusic, {}, "stop playback", "STnP PLdBaK", "STnP PLdBaK" },
		{ BB::Action::Scenario::PhoneStopMusic, {}, "pause playback", "PeZ PLdBaK", "PeZ PLdBaK" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Forward } /*VoiceForwardRoutine*/, "move forward", "MoV FeRWkD", "FeRWkD" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Forward } /*VoiceForwardRoutine*/, "go forward", "Gb FeRWkD", "FeRWkD" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Forward } /*VoiceForwardRoutine*/, "drive forward", "DRiV FeRWkD", "FeRWkD" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Backward } /*VoiceBackwardRoutine*/, "move backward", "MoV BaKWkD", "BaKWkD", 0.24f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Backward } /*VoiceBackwardRoutine*/, "go backward", "Gb BaKWkD", "BaKWkD" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Backward } /*VoiceBackwardRoutine*/, "drive backward", "DRiV BaKWkD", "BaKWkD", 0.23f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Rotate } /*VoiceTurn180Routine*/, "turn around", "TkN ktND", "ktND" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Rotate } /*VoiceTurn180Routine*/, "turn 180", "TkN WcN dTm", "WcN dTm" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Rotate } /*VoiceTurn180Routine*/, "rotate 180", "RbTdT WcN dTm", "WcN dTm" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Rotate } /*VoiceTurn180Routine*/, "spin around", "SPgN ktND", "ktND" },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Left } /*VoiceTurnLeftRoutine*/, "turn left", "TkN LfFT", "LfFT", 0.25f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Left } /*VoiceTurnLeftRoutine*/, "go left", "Gb LfFT", "LfFT", 0.21f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Left } /*VoiceTurnLeftRoutine*/, "rotate left", "RbTdT LfFT", "LfFT", 0.21f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Right } /*VoiceTurnRightRoutine*/, "turn right", "TkN RiT", "RiT", 0.21f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Right } /*VoiceTurnRightRoutine*/, "go right", "Gb RiT", "RiT", 0.21f },
		{ BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Right } /*VoiceTurnRightRoutine*/, "rotate right", "RbTdT RiT", "RiT", 0.21f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "who is this", "ho gZ jgS", "ho", 0.15f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "who is that", "ho gZ jaT", "ho", 0.15f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "who do you see", "ho Do Yo Sm", "ho Sm", 0.25f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "identify this person", "iDfNTcFi jgS PkScN", "iDfNTcFi PkScN" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "identify face", "iDfNTcFi FdS", "iDfNTcFi FdS" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "do you know this person", "Do Yo Nb jgS PkScN", "PkScN", 0.25f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "recognize this person", "RfKcGNiZ jgS PkScN", "RfKcGNiZ PkScN", 0.25f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } /*WhoThisRoutine*/, "who is in front of you", "ho gZ gN FRcNT cV Yo", "FRcNT" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "remember this face", "RgMfMBk jgS FdS", "RgMfMBk FdS" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "learn this face", "LkN jgS FdS", "LkN FdS" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "save this face", "SdV jgS FdS", "SdV FdS" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "set as owner", "SfT aZ bNk", "SfT bNk" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "this is my face", "jgS gZ Mi FdS", "jgS FdS" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "recognize me as owner", "RfKcGNiZ Mm aZ bNk", "RfKcGNiZ bNk" },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "register this face", "RfqgSTk jgS FdS", "RfqgSTk FdS", 0.23f },
		{ BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } /*RememberFaceRoutine*/, "make me the owner", "MdK Mm jc bNk", "MdK bNk" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "forget owner", "FkGfT bNk", "FkGfT bNk", 0.3f },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "remove owner", "RmMoV bNk", "RmMoV bNk" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "delete owner", "DgLmT bNk", "DgLmT bNk" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "forget creator", "FkGfT KRmdTk", "FkGfT KRmdTk", 0.3f },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "remove creator", "RmMoV KRmdTk", "RmMoV KRmdTk", 0.3f },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "delete creator", "DgLmT KRmdTk", "DgLmT KRmdTk" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "clear saved face", "KLgR SdVD FdS", "KLgR FdS" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "remove stored face", "RmMoV STeRD FdS", "RmMoV FdS" },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "forget my face", "FkGfT Mi FdS", "FkGfT FdS", 0.21f },
		{ BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget }, "delete saved owner", "DgLmT SdVD bNk", "DgLmT bNk" },
		{ BB::Action::Scenario::Dance, {}, "show me your moves", "sb Mm YeR MoVZ", "MoVZ", 0.3f },
		{ BB::Action::Scenario::Dance, {}, "do a dance", "Do c DaNS", "DaNS" },
		{ BB::Action::Scenario::Dance, {}, "move it, move it", "MoV gT MoV gT", "MoV" },
		{ BB::Action::Scenario::Dance, {}, "i want to see you dance", "i WnNT To Sm Yo DaNS", "DaNS" },
		{ BB::Action::Scenario::Dance, {}, "get your groove on", "GfT YeR GRoV nN", "GRoV" },
		{ BB::Action::Scenario::Dance, {}, "bust a move", "BcST c MoV", "BcST MoV" },
		{ BB::Action::Scenario::Dance, {}, "show me what you got", "sb Mm WcT Yo GnT", "", 0.22f },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::None }, "activity mode", "aKTgVcTm MbD", "aKTgVcTm MbD" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::None }, "idle activity", "iDcL aKTgVcTm", "iDcL aKTgVcTm" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Off }, "activity off", "aKTgVcTm eF", "aKTgVcTm eF" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Chill }, "activity chill", "aKTgVcTm pgL", "aKTgVcTm pgL" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Active }, "activity active", "aKTgVcTm aKTgV", "aKTgVcTm aKTgV" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Hyper }, "activity hyper", "aKTgVcTm hiPk", "aKTgVcTm hiPk" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Chill }, "be chill", "Bm pgL", "Bm pgL" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Active }, "be active", "Bm aKTgV", "Bm aKTgV" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Hyper }, "be hyper", "Bm hiPk", "Bm hiPk" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Chill }, "calm down", "KnM DtN", "KnM DtN" },
		{ BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Hyper }, "go crazy", "Gb KRdZm", "KRdZm" },
	}
);

constexpr int ActivationsNum = (int) activations.size();

#endif //BUTTERBOTCTRL_FIRMWARE_SCENARIOS_H
