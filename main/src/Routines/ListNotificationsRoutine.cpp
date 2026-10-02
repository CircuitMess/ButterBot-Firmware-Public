#include "ListNotificationsRoutine.h"
#include <Statics/ApplicationStatics.h>
#include <Core/Application.h>
#include <Services/Audio/Audio.h>
#include <BBData.h>
#include <Phrases.h>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <Util/ServiceLocator.h>
#include "Audio/SpeechAudioGen.h"
#include "Audio/SpeechAudioSource.h"
#include "Audio/SpeechGen.h"
#include "Services/Com.h"
#include "Phone/Phone.h"
#include "Phone/Notif.h"

DEFINE_LOG(ListNotificationsRoutine)

static constexpr AudioFrontend::Phrase YesNoPhrases[] = {
    { "yes",            "YfS",		"YfS"           },
    { "no",             "Nb",			"Nb"            },
};

// Index 0 = "yes", index 1 = "no"
static constexpr int YesNoThreshold = 1;

static const std::set<std::string> KnownSocialApps = {
    "WhatsApp", "Messenger", "Viber", "Instagram", "iMessage", "Snapchat", "TikTok", "Messages"
};

static std::string callerIdentity(const std::string& title, const std::string& message){
    static const std::set<std::string> genericLabels = {
        "Missed call", "Missed Call",
        "Voicemail",
    };
    if(!title.empty() && genericLabels.count(title) == 0){
        return title;
    }
    if(!message.empty()){
        return message;
    }
    return "an unknown number";
}

static std::string formatNotifText(const Notif& notif){
    const std::string& title = notif.title;
    const std::string& message = notif.message;

    switch(notif.category){
        case Notif::Category::MissedCall:
            return "Missed call from " + callerIdentity(title, message);

        case Notif::Category::Voicemail:
            return "Voicemail from " + callerIdentity(title, message);

        case Notif::Category::Email:
            return "Email from " + title + ": " + message;

        case Notif::Category::News:
            return "News from " + notif.appID + ": " + message;

        case Notif::Category::Schedule:
            return "Reminder: " + message;

        case Notif::Category::Social:
            if(KnownSocialApps.count(notif.appID)){
                return title + " on " + notif.appID + " says: " + message;
            }
            return title + " says: " + message;

        case Notif::Category::HealthAndFitness:
        case Notif::Category::BusinessAndFinance:
        case Notif::Category::Location:
        case Notif::Category::Entertainment:
        case Notif::Category::Other:
        default:
            return "App " + notif.appID + " says: " + message;
    }
}

static void playText(Audio* audio, const std::string& text){
    auto source = std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, text);
    audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::move(source));
    audio->waitEnd(portMAX_DELAY);
}

static void playPhrase(Audio* audio, Phrase phrase){
    const int16_t id = Phrases::get(phrase);
    if(id < 0){
        CMF_LOG(ListNotificationsRoutine, LogLevel::Warning, "No phrase output for phrase %d", (int)phrase);
        return;
    }
    playText(audio, Phrases::map(phrase, id));
}

static void sendListNotifsState(Com* com, PhoneListNotifsData::Phase phase, uint8_t id, uint8_t count, uint8_t remaining){
    PhoneListNotifsData bbData{};
    bbData.phase = phase;
    bbData.id = id;
    bbData.count = count;
    bbData.remaining = remaining;
    com->sendData(BB::State::Scenario, BB::Action::Scenario::PhoneListNotifs, bbData);
}

Routine::TickingState ListNotificationsRoutine::tick(float deltaTime){
    const Application* app = ApplicationStatics::getApplication();
    Audio* audio = app->getService<Audio>();
    Com* com = app->getService<Com>();
    Phone* phone = app->getService<Phone>();

    if(!audio || !ServiceLocator::SpeechAudioGenInstance || !ServiceLocator::SpeechGenInstance || !com || !af || !phone){
        CMF_LOG(ListNotificationsRoutine, LogLevel::Error, "Missing required service(s)");
        return TickingState::Done;
    }

    switch(phase){
        case Phase::Init: {
            if(!phone->isConnected()){
                const int16_t id = Phrases::get(Phrase::PhoneNotConnected);
                if(id >= 0){
                    PhoneNotConnectedData bbData{};
                    bbData.id = static_cast<uint8_t>(id);
                    com->sendData(BB::State::Scenario, BB::Action::Scenario::PhoneNotConnected, bbData);
                    playText(audio, Phrases::map(Phrase::PhoneNotConnected, id));
                }
                return TickingState::Done;
            }

            // Izbacivanje duplikata - TODO: ovo je fix za Phone razred i dupliciranje, imamo task na Jiri
            notifs = phone->getNotifs();
            size_t write = 0;
            for(size_t read = 0; read < notifs.size(); ++read){
                const Notif& candidate = notifs[read];

                // IncomingCall and OutgoingCall are not real notifications — skip them entirely.
                if(candidate.category == Notif::Category::IncomingCall) continue;
                if(candidate.category == Notif::Category::OutgoingCall) continue;

                // MissedCall and Voicemail carry no message body — exempt them from the
                // empty-message filter that otherwise skips Android group-summary / badge-only entries.
                const bool isCallCategory = candidate.category == Notif::Category::MissedCall
                                         || candidate.category == Notif::Category::Voicemail;
                if(candidate.message.empty() && !isCallCategory) continue;

                const bool duplicate = std::any_of(notifs.cbegin(), notifs.cbegin() + write, [&candidate](const Notif& existing){
                    return existing.appID == candidate.appID
                           && existing.title == candidate.title
                           && existing.message == candidate.message;
                });
                if(duplicate) continue;

                if(write != read){
                    notifs[write] = std::move(notifs[read]);
                }
                ++write;
            }
            notifs.erase(notifs.begin() + write, notifs.end());
            ///////////// Izbacivanje duplikata //////////////
            count = static_cast<uint8_t>(notifs.size() > 255 ? 255 : notifs.size());

            if(count == 0){
                sendListNotifsState(com, PhoneListNotifsData::Phase::NoNotifs, 0, 0, 0);
                playPhrase(audio, Phrase::PhoneNoNotifs);
                return TickingState::Done;
            }

            const int16_t countId = Phrases::get(Phrase::PhoneNotifCount);
            sendListNotifsState(com, PhoneListNotifsData::Phase::Count, (countId >= 0) ? static_cast<uint8_t>(countId) : 0, count, count);

            const std::string countText = (count == 1)
                ? "You have 1 notification."
                : "You have " + std::to_string(count) + " notifications.";
            playText(audio, countText);

            CMF_LOG(ListNotificationsRoutine, LogLevel::Info, "Reading %d notifications", (int)count);
            index = 0;
            phase = Phase::Reading;
            return TickingState::Continue;
        }

        case Phase::Reading: {
            const Notif& notif = notifs[index];
            const uint8_t remaining = count - index - 1;

            sendListNotifsState(com, PhoneListNotifsData::Phase::Reading, 0, count, remaining);
            playText(audio, formatNotifText(notif));

            // Every 5 notifications, ask if the user wants to continue.
            if(remaining > 0 && (index + 1) % 5 == 0){
                const int16_t continueId = Phrases::get(Phrase::PhoneAskContinue);
                if(continueId >= 0){
                    sendListNotifsState(com, PhoneListNotifsData::Phase::AskContinue, static_cast<uint8_t>(continueId), count, remaining);

                    const std::string continueTemplateText = Phrases::map(Phrase::PhoneAskContinue, continueId);
                    char buf[128];
                    snprintf(buf, sizeof(buf), continueTemplateText.c_str(), static_cast<int>(remaining));
                    playText(audio, std::string(buf));

                    listen(YesNoPhrases);
                    phase = Phase::WaitContinue;
                    return TickingState::Block;
                }
            }

            ++index;
            phase = (index >= count) ? Phase::Finish : Phase::Reading;
            return TickingState::Continue;
        }

        case Phase::WaitContinue: {
            if(!phraseReady()){
                return TickingState::Block;
            }

            const AudioFrontend::PhraseResult& result = phraseResult();
            const bool stop = !result.recognized || result.index < 0 || result.index >= YesNoThreshold;

            if(result.index < 0){
                if(ServiceLocator::ScenarioRoutineServiceInstance){
                    audio->stop();
                    audio->play(ServiceLocator::SpeechAudioGenInstance.get(), std::make_unique<SpeechAudioSource>(SpeechGen::InputType::Text, Phrases::map(Phrase::ListenAbort, Phrases::get(Phrase::ListenAbort))));
                }
            }

            if(stop){
                CMF_LOG(ListNotificationsRoutine, LogLevel::Info, "User stopped reading at notification %d", (int)(index + 1));
                phase = Phase::Finish;
                return TickingState::Continue;
            }

            ++index;
            phase = (index >= count) ? Phase::Finish : Phase::Reading;
            return TickingState::Continue;
        }

        case Phase::Finish: {
            sendListNotifsState(com, PhoneListNotifsData::Phase::Done, 0, count, 0);
            playPhrase(audio, Phrase::PhoneAllRead);
            return TickingState::Done;
        }
    }

    return TickingState::Done;
}
