#include "SettingsActivityRoutine.h"
#include <Core/Application.h>
#include <Statics/ApplicationStatics.h>
#include <Services/Audio/Audio.h>
#include <BBData.h>
#include <Phrases.h>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <Util/ServiceLocator.h>
#include "Audio/SpeechAudioGen.h"
#include "Audio/SpeechAudioSource.h"
#include "Audio/SpeechGen.h"
#include "Services/Com.h"
#include "Services/Settings.h"

DEFINE_LOG(SettingsActivityRoutine);

struct LevelEntry {
	AudioFrontend::Phrase phrase;
	ActivityMode level;
};

static constexpr LevelEntry LevelTable[] = {
	{ { "off", "eF", "eF" }, ActivityMode::Off },
	{ { "turn it off", "TkN gT eF", "eF" }, ActivityMode::Off },
	{ { "chill", "pgL", "pgL" }, ActivityMode::Chill },
	{ { "active", "aKTgV", "aKTgV" }, ActivityMode::Active },
	{ { "hyper", "hiPk", "hiPk" }, ActivityMode::Hyper },
};

static constexpr Phrase LevelPhrases[] = { Phrase::SettingsActivityOff, Phrase::SettingsActivityChill, Phrase::SettingsActivityActive, Phrase::SettingsActivityHyper };

static std::vector<AudioFrontend::Phrase> levelPhraseList(){
	std::vector<AudioFrontend::Phrase> list;
	list.reserve(std::size(LevelTable));
	for(const LevelEntry& entry : LevelTable){
		list.push_back(entry.phrase);
	}
	return list;
}

static void speakText(Audio* audio, std::string text){
	std::unique_ptr<SpeechAudioSource> source = std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, std::move(text));
	audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::move(source));
	audio->waitEnd(portMAX_DELAY);
}

static void playPhrase(Audio* audio, Com* com, Phrase phrase){
	const int16_t id = Phrases::get(phrase);
	if(id < 0){
		CMF_LOG(SettingsActivityRoutine, LogLevel::Warning, "No phrase output for phrase id %d", (int) phrase);
		return;
	}

	com->sendData(BB::State::Scenario, BB::Action::Scenario::SettingsActivity, SettingsChangeData{ .phrase = phrase, .id = static_cast<uint8_t>(id) });
	speakText(audio, Phrases::map(phrase, id));
}

static void handleNoInput(Audio* audio, Com* com, const AudioFrontend::PhraseResult& res){
	if(res.recognized){
		audio->stop();
		audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, Phrases::map(Phrase::ListenAbort, Phrases::get(Phrase::ListenAbort))));
		CMF_LOG(SettingsActivityRoutine, LogLevel::Info, "Level: aborted by user");
		return;
	}

	if(res.transcript.empty()){
		CMF_LOG(SettingsActivityRoutine, LogLevel::Warning, "Level: timeout (no speech detected)");
	} else{
		CMF_LOG(SettingsActivityRoutine, LogLevel::Warning, "Level: unrecognized speech '%s'", res.transcript.c_str());
	}

	playPhrase(audio, com, Phrase::ListeningTimeout);
}

static void applyLevel(Audio* audio, Com* com, ActivityMode level){
	if(ServiceLocator::SettingsInstance){
		SettingsStruct settings = ServiceLocator::SettingsInstance->get();
		settings.activityMode = level;
		ServiceLocator::SettingsInstance->set(settings);
		ServiceLocator::SettingsInstance->store();

		com->sendData(BB::State::Idle, BB::Action::Idle::SettingsSync, SettingsSyncData{ .frontSensor = settings.frontSensor, .bottomSensor = settings.bottomSensor, .activityMode = settings.activityMode });
	}

	playPhrase(audio, com, LevelPhrases[static_cast<size_t>(level)]);
}

Routine::TickingState SettingsActivityRoutine::tick(float deltaTime){
	const Application* app = ApplicationStatics::getApplication();
	Audio* audio = app->getService<Audio>();
	Com* com = app->getService<Com>();

	if(audio == nullptr || !ServiceLocator::SpeechAudioGenInstance || !ServiceLocator::SpeechGenInstance || com == nullptr || af == nullptr){
		CMF_LOG(SettingsActivityRoutine, LogLevel::Error, "Missing required service(s)");
		return TickingState::Done;
	}

	switch(phase){
		case Phase::Init: {
			if(levelKnown){
				applyLevel(audio, com, knownLevel);
				return TickingState::Done;
			}

			playPhrase(audio, com, Phrase::SettingsAskActivity);

			const std::vector<AudioFrontend::Phrase> phrases = levelPhraseList();
			listen(phrases);
			phase = Phase::WaitLevel;
			return TickingState::Block;
		}

		case Phase::WaitLevel: {
			if(!phraseReady()){
				return TickingState::Block;
			}

			const AudioFrontend::PhraseResult& result = phraseResult();
			if(result.index == -1){
				handleNoInput(audio, com, result);
				return TickingState::Done;
			}

			af->setMode(AudioFrontend::Mode::Off);
			af->unloadModel();

			applyLevel(audio, com, LevelTable[result.index].level);
			return TickingState::Done;
		}
	}

	return TickingState::Done;
}

SettingsActivityOffRoutine::SettingsActivityOffRoutine(BBStateMachine* sm) : SettingsActivityRoutine(sm){
	levelKnown = true;
	knownLevel = ActivityMode::Off;
}

SettingsActivityChillRoutine::SettingsActivityChillRoutine(BBStateMachine* sm) : SettingsActivityRoutine(sm){
	levelKnown = true;
	knownLevel = ActivityMode::Chill;
}

SettingsActivityActiveRoutine::SettingsActivityActiveRoutine(BBStateMachine* sm) : SettingsActivityRoutine(sm){
	levelKnown = true;
	knownLevel = ActivityMode::Active;
}

SettingsActivityHyperRoutine::SettingsActivityHyperRoutine(BBStateMachine* sm) : SettingsActivityRoutine(sm){
	levelKnown = true;
	knownLevel = ActivityMode::Hyper;
}