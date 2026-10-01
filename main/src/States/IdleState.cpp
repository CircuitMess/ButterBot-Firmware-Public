#include "IdleState.h"
#include <Core/Application.h>
#include <Statics/ApplicationStatics.h>
#include <Routines/MotionRoutine.h>
#include <Services/MotionService.h>
#include <Util/ServiceLocator.h>
#include "BBStateMachine.h"
#include "Enums.h"
#include "ListenState.h"
#include "ScenarioState.h"
#include "ScenarioRoutineMappings.h"
#include "Services/Com.h"
#include "RCState.h"
#include "Routines/RambleRoutine.h"
#include "Routines/ObserveRoutine.h"
#include "Routines/PersonRoutine.h"
#include "Routines/EventRoutines/BatteryEventRoutine.h"
#include "Routines/EventRoutines/GasEventRoutine.h"
#include "Routines/EventRoutines/IntruderEventRoutine.h"
#include "Routines/EventRoutines/ModuleEventRoutine.h"
#include "Routines/EventRoutines/PhoneConnEventRoutine.h"
#include "Routines/EventRoutines/PokeEventRoutine.h"
#include "Routines/EventRoutines/GasConfiguredEventRoutine.h"
#include "Routines/EventRoutines/SummonEventRoutine.h"
#include "Routines/WanderRoutine.h"
#include "Util/Roaming.h"

DEFINE_LOG(IdleState)

const std::vector<IdleState::RandomRoutineDef> IdleState::RandomRoutines = {
	{ &makeRoutine<RambleRoutine>, 3 },
	{ &makeRoutine<ObserveRoutine>, 1 },
	{ &makeRoutine<WanderRoutine>, 2 },
	{ &makeRoutine<PersonRoutine>, 1 },
};

const std::array<EventRoutineFactory, static_cast<uint8_t>(EventBag::EventType::COUNT)> IdleState::EventRoutines = {
	nullptr, // None
	nullptr, // IncomingCall
	&makeEventRoutine<BatteryEventRoutine>,
	nullptr, // Notification
	&makeEventRoutine<GasEventRoutine>,
	&makeEventRoutine<IntruderEventRoutine>,
	&makeEventRoutine<ModuleEventRoutine>,
	&makeEventRoutine<ModuleEventRoutine>,
	&makeEventRoutine<GasConfiguredEventRoutine>,
	&makeEventRoutine<PhoneConnEventRoutine>,
	&makeEventRoutine<SummonEventRoutine>,
	nullptr, // Scenario
};

IdleState::IdleState(BBStateMachine* sm) : State(sm){
	nextRandomRoutineTime = millis() + MinRandomDelay;

	const Application* app = ApplicationStatics::getApplication();
	eventBag = app->getService<EventBag>();
	if(eventBag == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "EventBag service missing in constructor");
		return;
	}

	ISRButtonInput* buttonInput = app->getService<ISRButtonInput>();
	if(buttonInput == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "ButtonInput service missing in constructor");
		return;
	}

	sm->bindState(buttonInput->OnButtonEvent, this, &IdleState::onButtonEvent);

	com = app->getService<Com>();
	if(com == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "Com service missing in constructor");
		return;
	}

	sm->bindState(com->OnCommand, this, &IdleState::onCommand);
	sm->bindState(com->OnRCData, this, &IdleState::onRCData);

	auto motionService = app->getService<MotionService>();
	if(motionService == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "MotionService missing in constructor");
		return;
	}

	motionService->enable();

	// Motion and Poke are not queued in the EventBag; they are handled the instant they arrive,
	// dropped while a routine is running, and dropped for StaleEventWindow after a routine ends
	// (events broadcast during a blocking routine are drained right after it retires - see
	// endActiveRoutine). Both callbacks run on the StateMachine thread.
	sm->bindState(motionService->onMotion, this, &IdleState::onMotion);

	if(!eventBag->probe()){
		com->sendData(BB::State::Idle, BB::Action::Idle::None);
	}
}

IdleState::~IdleState(){
	// The active routine and event bindings are torn down by BBStateMachine::destroyState().
	const Application* app = ApplicationStatics::getApplication();

	if(MotionService* motionService = app->getService<MotionService>()){
		motionService->disable();
	}
}

void IdleState::tick(float deltaTime){
	if(sm->isTransitionPending()){
		return;
	}

	if(eventBag == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "Event Bag is nullptr.");
		return;
	}

	const bool eventPending = eventBag->probe();

	// A running random routine yields to a pending event so the event is handled now.
	if(sm->getActiveRoutine() != nullptr && shouldYieldToEvent(eventPending)){
		endActiveRoutine();
	}

	// Start a new routine if nothing is active. Pending events take priority over random routines.
	if(sm->getActiveRoutine() == nullptr){
		if(eventPending){
			startEventRoutine(eventBag->pop());
		} else{
			maybeStartRandomRoutine();
		}
	}

	// Tick whatever is active now - including a routine created during this same tick -
	// and retire it the moment it reports completion.
	if(Routine* routine = sm->getActiveRoutine()){
		lastTickingState = routine->tick(deltaTime);

		if(lastTickingState == Routine::TickingState::Done){
			endActiveRoutine();
		}
	}
}

bool IdleState::shouldYieldToEvent(bool eventPending) const{
	// Only random (non-event) routines yield; an active EventRoutine runs to completion.
	const Routine* routine = sm->getActiveRoutine();
	return routine != nullptr && !routine->isEventRoutine() && eventPending;
}

void IdleState::endActiveRoutine(){
	sm->endRoutine();

	// Poke/Motion events broadcast while the routine blocked this thread sit in their CMF callQueues
	// and get drained on the very next scanEvents pass, where the canStartInstantRoutine() guard would
	// let the first one through. They arrive within ~1ms of this point; anything handled inside the
	// window is stale and dropped.
	instantEventCutoff = millis() + StaleEventWindow;

	lastTickingState = Routine::TickingState::None;

	com->sendData(BB::State::Idle, BB::Action::Idle::None);
}

void IdleState::startEventRoutine(const EventBag::EventData& data){
	if(data.type == EventBag::EventType::Scenario){
		startScenario(data);
		return;
	}

	// Some event types (e.g. IncomingCall, Notification) have no routine mapped. Drop them instead of constructing a null routine.
	const EventRoutineFactory factory = EventRoutines[static_cast<uint8_t>(data.type)];
	if(factory == nullptr){
		CMF_LOG(IdleState, LogLevel::Warning, "No EventRoutine mapped for event type %u", static_cast<uint8_t>(data.type));
		com->sendData(BB::State::Idle, BB::Action::Idle::None);
		rescheduleRandomRoutine();
		return;
	}

	// Each routine is implemented to take only one parameter of type EventBag::EventData, that it then interprets in its own way.
	sm->startEventRoutine(factory, data);

	rescheduleRandomRoutine();
}

void IdleState::startScenario(const EventBag::EventData& data){
	const EventBag::ScenarioEventData* scenarioEvent = std::get_if<EventBag::ScenarioEventData>(&data.payload);
	if(scenarioEvent == nullptr || !ServiceLocator::ScenarioRoutineServiceInstance){
		CMF_LOG(IdleState, LogLevel::Error, "Scenario event missing payload or ScenarioRoutineService");
		com->sendData(BB::State::Idle, BB::Action::Idle::None);
		rescheduleRandomRoutine();
		return;
	}

	const RoutineFactory factory = routineForScenario({ scenarioEvent->scenario, scenarioEvent->data });
	if(factory == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "No routine mapped for scenario %d", static_cast<int>(scenarioEvent->scenario));
		com->sendData(BB::State::Idle, BB::Action::Idle::None);
		rescheduleRandomRoutine();
		return;
	}

	ServiceLocator::ScenarioRoutineServiceInstance->setRoutineFactory(factory);
	sm->transitionTo<ScenarioState>();
}

void IdleState::maybeStartRandomRoutine(){
	if(RandomRoutines.empty()){
		rescheduleRandomRoutine();
		return;
	}

	// While the button is held, suppress new random routines so none start before the transition to ListenState commits on release.
	if(buttonHeld){
		return;
	}

	if(millis() < nextRandomRoutineTime){
		return;
	}

	RoutineFactory nextRandomRoutine = pickRandomRoutine();
	if(nextRandomRoutine != nullptr){
		sm->startRoutine(nextRandomRoutine);
	}

	rescheduleRandomRoutine();
}

RoutineFactory IdleState::pickRandomRoutine() const{
	// With roaming off the robot doesn't wander (PersonRoutine stays, it just doesn't move)
	const auto weightOf = [](const RandomRoutineDef& def) -> size_t {
		if(def.routine == &makeRoutine<WanderRoutine> && !Roaming::enabled) return 0;
		return def.weight;
	};

	size_t weightSum = 0;
	for(const RandomRoutineDef& routineDef : RandomRoutines){
		weightSum += weightOf(routineDef);
	}

	const size_t random = rand() % (weightSum + 1);

	size_t sum = 0;
	for(const RandomRoutineDef& routineDef : RandomRoutines){
		if(weightOf(routineDef) == 0){
			continue;
		}

		sum += weightOf(routineDef);

		if(random <= sum){
			return routineDef.routine;
		}
	}

	return nullptr;
}

void IdleState::rescheduleRandomRoutine(){
	nextRandomRoutineTime = millis() + (rand() % (MaxRandomDelay - MinRandomDelay) + MinRandomDelay);
}

int64_t IdleState::getDynamicTickInterval() const{
	if(eventBag == nullptr){
		CMF_LOG(IdleState, LogLevel::Error, "Event Bag is nullptr.");
		return -1;
	}

	if(eventBag->probe()){
		return 0;
	}

	if(sm->getActiveRoutine() != nullptr){
		if(lastTickingState == Routine::TickingState::Continue){
			return 0;
		}

		// Block: sleep until the awaited event wakes the tick thread.
		if(lastTickingState == Routine::TickingState::Block){
			return portMAX_DELAY;
		}
	}

	if(millis() >= nextRandomRoutineTime){
		return 0;
	}

	return nextRandomRoutineTime - millis();
}

void IdleState::onCommand(Ctrl::Command cmd){
	if(cmd == Ctrl::Command::Listen){
		sm->transitionTo<ListenState>();
		return;
	}

	if(cmd == Ctrl::Command::Poke && millis() >= instantEventCutoff && canStartInstantRoutine()){
		sm->startRoutine<PokeEventRoutine>();
	}
}

void IdleState::onMotion(MotionType type){
	if(millis() >= instantEventCutoff && canStartInstantRoutine()){
		sm->startRoutine<MotionRoutine>(type);
	}
}

bool IdleState::canStartInstantRoutine() const{
	return sm->getActiveRoutine() == nullptr && !sm->isTransitionPending();
}

void IdleState::onRCData(const RCData& data){
	if(ServiceLocator::ControllerStateInstance){
		ServiceLocator::ControllerStateInstance->updateRCData(data);
	}

	sm->transitionTo<RCState>();
	return;
}

void IdleState::onButtonEvent(int button, ISRButtonInput::Action action){
	if(button != static_cast<int>(Button::Power)){
		return;
	}

	if(action == ISRButtonInput::Action::Press){
		buttonHeld = true;
		lastBtnPressTime = millis();
		return;
	}

	buttonHeld = false;

	if(millis() - lastBtnPressTime > MaximumBtnHoldForTransition){
		return;
	}

	sm->transitionTo<ListenState>();
}
