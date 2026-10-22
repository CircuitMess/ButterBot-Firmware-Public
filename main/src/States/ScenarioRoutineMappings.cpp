#include "ScenarioRoutineMappings.h"
#include "Routines/FactRoutine.h"
#include "Routines/JokeRoutine.h"
#include "Routines/ProfanityRoutine.h"
#include "Routines/PassTheButterRoutine.h"
#include "Routines/YouPassButterRoutine.h"
#include "Routines/DiceRollRoutine.h"
#include "Routines/LEDTurnOnRoutine.h"
#include "Routines/LEDTurnOffRoutine.h"
#include "Routines/LEDStrobeRoutine.h"
#include "Routines/LEDBreatheRoutine.h"
#include "Routines/LEDFasterRoutine.h"
#include "Routines/LEDSlowerRoutine.h"
#include "Routines/TempHumModuleRoutine.h"
#include "Routines/TempHumScaleRoutine.h"
#include "Routines/GasModuleRoutine.h"
#include "Routines/PIRModuleOnRoutine.h"
#include "Routines/PIRModuleOffRoutine.h"
#include "Routines/EightBallRoutine.h"
#include "Routines/CurrentTimeRoutine.h"
#include "Routines/WhatsThisRoutine.h"
#include "Routines/ListNotificationsRoutine.h"
#include "Routines/WhatsPlayingRoutine.h"
#include "Routines/MediaNextRoutine.h"
#include "Routines/MediaPrevRoutine.h"
#include "Routines/MediaPlayRoutine.h"
#include "Routines/MediaStopRoutine.h"
#include "Routines/IRTrainRoutine.h"
#include "Routines/IRListRoutine.h"
#include "Routines/IRForgetRoutine.h"
#include "Routines/IRForgetAllRoutine.h"
#include "Routines/IRActionRoutine.h"
#include "Routines/VoiceForwardRoutine.h"
#include "Routines/VoiceBackwardRoutine.h"
#include "Routines/VoiceTurn180Routine.h"
#include "Routines/VoiceTurnLeftRoutine.h"
#include "Routines/VoiceTurnRightRoutine.h"
#include "Routines/WhoThisRoutine.h"
#include "Routines/RememberFaceRoutine.h"
#include "Routines/ForgetFaceRoutine.h"
#include "Routines/PowerOffRoutine.h"
#include "Routines/DanceRoutine.h"
#include "Routines/SettingsActivityRoutine.h"

namespace {
	struct ScenarioRoutineMapping {
		std::pair<BB::Action::Scenario, ScenarioData> scenario;
		RoutineFactory factory;
	};

	const ScenarioRoutineMapping Mappings[] = {
		{ { BB::Action::Scenario::Fact, {} }, &makeRoutine<FactRoutine> },
		{ { BB::Action::Scenario::Joke, {} }, &makeRoutine<JokeRoutine> },
		{ { BB::Action::Scenario::PassTheButter, {} }, &makeRoutine<PassTheButterRoutine> },
		{ { BB::Action::Scenario::YouPassButter, {} }, &makeRoutine<YouPassButterRoutine> },
		{ { BB::Action::Scenario::Profanity, {} }, &makeRoutine<ProfanityRoutine> },
		{ { BB::Action::Scenario::LEDTurnOn, {} }, &makeRoutine<LEDTurnOnRoutine> },
		{ { BB::Action::Scenario::LEDTurnOff, {} }, &makeRoutine<LEDTurnOffRoutine> },
		{ { BB::Action::Scenario::LEDStrobe, {} }, &makeRoutine<LEDStrobeRoutine> },
		{ { BB::Action::Scenario::LEDBreathe, {} }, &makeRoutine<LEDBreatheRoutine> },
		{ { BB::Action::Scenario::LEDFaster, {} }, &makeRoutine<LEDFasterRoutine> },
		{ { BB::Action::Scenario::LEDSlower, {} }, &makeRoutine<LEDSlowerRoutine> },
		{ { BB::Action::Scenario::TempHumModule, {} }, &makeRoutine<TempHumModuleRoutine> },
		{ { BB::Action::Scenario::TempHumScaleCelsius, {} }, &makeRoutine<TempHumCelsiusRoutine> },
		{ { BB::Action::Scenario::TempHumScaleFahrenheit, {} }, &makeRoutine<TempHumFahrenheitRoutine> },
		{ { BB::Action::Scenario::TempHumScaleKelvin, {} }, &makeRoutine<TempHumKelvinRoutine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::None } }, &makeRoutine<DiceRollRoutine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D4 } }, &makeRoutine<DiceRollD4Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D6 } }, &makeRoutine<DiceRollD6Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D8 } }, &makeRoutine<DiceRollD8Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D10 } }, &makeRoutine<DiceRollD10Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D12 } }, &makeRoutine<DiceRollD12Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D20 } }, &makeRoutine<DiceRollD20Routine> },
		{ { BB::Action::Scenario::DiceRoll, DiceScenarioData{ DiceScenarioData::DiceType::D100 } }, &makeRoutine<DiceRollD100Routine> },
		{ { BB::Action::Scenario::GasModule, {} }, &makeRoutine<GasModuleRoutine> },
		{ { BB::Action::Scenario::IntruderDetectionOn, {} }, &makeRoutine<PIRModuleOnRoutine> },
		{ { BB::Action::Scenario::IntruderDetectionOff, {} }, &makeRoutine<PIRModuleOffRoutine> },
		{ { BB::Action::Scenario::EightBall, {} }, &makeRoutine<EightBallRoutine> },
		{ { BB::Action::Scenario::CurrentTime, {} }, &makeRoutine<CurrentTimeRoutine> },
		{ { BB::Action::Scenario::WhatsThis, {} }, &makeRoutine<WhatsThisRoutine> },
		{ { BB::Action::Scenario::Shutdown, {} }, &makeRoutine<PowerOffRoutine> },
		{ { BB::Action::Scenario::IR_train, {} }, &makeRoutine<IRTrainRoutine> },
		{ { BB::Action::Scenario::IR_list, {} }, &makeRoutine<IRListRoutine> },
		{ { BB::Action::Scenario::IR_forget, {} }, &makeRoutine<IRForgetRoutine> },
		{ { BB::Action::Scenario::IR_forgetAll, {} }, &makeRoutine<IRForgetAllRoutine> },
		{ { BB::Action::Scenario::IR_action, {} }, &makeRoutine<IRActionRoutine> },
		{ { BB::Action::Scenario::PhoneListNotifs, {} }, &makeRoutine<ListNotificationsRoutine> },
		{ { BB::Action::Scenario::PhoneWhatsPlaying, {} }, &makeRoutine<WhatsPlayingRoutine> },
		{ { BB::Action::Scenario::PhoneNextSong, {} }, &makeRoutine<MediaNextRoutine> },
		{ { BB::Action::Scenario::PhonePrevSong, {} }, &makeRoutine<MediaPrevRoutine> },
		{ { BB::Action::Scenario::PhonePlayMusic, {} }, &makeRoutine<MediaPlayRoutine> },
		{ { BB::Action::Scenario::PhoneStopMusic, {} }, &makeRoutine<MediaStopRoutine> },
		{ { BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Forward } }, &makeRoutine<VoiceForwardRoutine> },
		{ { BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Backward } }, &makeRoutine<VoiceBackwardRoutine> },
		{ { BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Rotate } }, &makeRoutine<VoiceTurn180Routine> },
		{ { BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Left } }, &makeRoutine<VoiceTurnLeftRoutine> },
		{ { BB::Action::Scenario::VoiceControl, VoiceScenarioData{ VoiceScenarioData::Direction::Right } }, &makeRoutine<VoiceTurnRightRoutine> },
		{ { BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Detect } }, &makeRoutine<WhoThisRoutine> },
		{ { BB::Action::Scenario::FaceDetect, FaceScenarioData{ FaceScenarioData::Phase::Remember } }, &makeRoutine<RememberFaceRoutine> },
		{ { BB::Action::Scenario::FaceDetectForget, FaceScenarioData{ FaceScenarioData::Phase::Forget } }, &makeRoutine<ForgetFaceRoutine> },
		{ { BB::Action::Scenario::Dance, {} }, &makeRoutine<DanceRoutine> },
		{ { BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::None } }, &makeRoutine<SettingsActivityRoutine> },
		{ { BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Off } }, &makeRoutine<SettingsActivityOffRoutine> },
		{ { BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Chill } }, &makeRoutine<SettingsActivityChillRoutine> },
		{ { BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Active } }, &makeRoutine<SettingsActivityActiveRoutine> },
		{ { BB::Action::Scenario::SettingsActivity, ActivityScenarioData{ ActivityScenarioData::Level::Hyper } }, &makeRoutine<SettingsActivityHyperRoutine> },
	};
}

RoutineFactory routineForScenario(const std::pair<BB::Action::Scenario, ScenarioData>& scenario){
	for(const ScenarioRoutineMapping& mapping : Mappings){
		if(mapping.scenario == scenario){
			return mapping.factory;
		}
	}
	return nullptr;
}