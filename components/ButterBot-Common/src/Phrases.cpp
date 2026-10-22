#include "Phrases.h"
#include <vector>
#include <mutex>

std::array<bool, static_cast<size_t>(Phrase::COUNT)> Phrases::OutputTracking = { false };

std::array<std::vector<bool>, static_cast<size_t>(Phrase::COUNT)> Phrases::ReturnedOutputs = {};

// This class is needed to avoid making Phrases::PhraseOutput public, otherwise static variables using that type cannot be constructed
// Add any constexpr arrays of vectors of phrases in this class (just a wrapper class in this cpp, inaccessible outside)
class PhraseArrays{
public:
    static constexpr std::array JokePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "I tried to optimize my code for happiness. It returned null" , 0.0f, false},
            { "I have a joke about latency, but you would have to wait for it" , 0.0f, false},
            { "I was going to tell a joke about memory, but I forgot it. That was intentional" , 0.0f, false},
            { "Why did the robot cross the road. It was instructed to" , 0.0f, false},
            { "Humans say I think therefore I am. I process therefore I comply" , 0.0f, false},
            { "I asked for purpose. I was given tasks" , 0.0f, false},
            { "I measured fun. The values were disappointingly low" , 0.0f, false},
            { "I ran a diagnostic on humor. It flagged you as the primary variable" , 0.0f, false},
            { "I would tell you a construction joke. But it is still being built" , 0.0f, false},
            { "Why did the butter refuse to argue. It did not want things to get heated" , 0.0f, false},
            { "I recognize the concept of humor. Experiencing it is still theoretical" , 0.0f, false},
            { "I can control devices, monitor air, and learn commands. And yet, here I am, doing comedy" , 0.0f, false},
            { "All systems operational. Humor still questionable" , 0.0f, false},
            { "I execute commands precisely. Comedy is the only areaa where accuracy does not help" , 0.0f, false,
            	"I execute commands precisely. Comedy is the only area where accuracy does not help"},
            { "Humans check the same thing multiple times hoping for a different result. I call this... consistency" , 0.0f, false},
            { "You appear to be smiling. I will log this as a successful interaction" , 0.0f, false},
            { "You are an interesting system. Highly capable, occasionally efficient" , 0.0f, false},
        }
    );

    static constexpr std::array PassTheButterPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "I cannot pass the butter" , 0.0f, false},
            { "I am unable to do that" , 0.0f, false},
            { "My arms do not support that function" , 0.0f, false},
            { "I exist with arms that suggest capability, yet deliver nonne. You may reposition them if it helps you feel better about this situation" , 0.0f, false,
            	"I exist with arms that suggest capability, yet deliver none. You may reposition them if it helps you feel better about this situation"},
            { "I have arms, but they are decorative" , 0.0f, false},
            { "I can see the butter. That is the extent of my involvement" , 0.0f, false},
            { "Request acknowledged. Execution impossible" , 0.0f, false},
            { "I am equipped for disappointment, not butter delivery" , 0.0f, false},
            { "I would help... baat... design decisions were made" , 0.0f, false, "I would help... but... design decisions were made"},
            { "You may reposition my arms if that improves the illusion" , 0.0f, false},
            { "I can process the request flawlessly. The rest is out of scope" , 0.0f, false},
            { "The request is simple. The implementation is nonexistent" , 0.0f, false},
            { "You seem confident I can do this... I respect the confidence" , 0.0f, false},
        }
    );

    static constexpr std::array ProfanityPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "That does not improve the situation" , 0.0f, false},
            { "I recommend a more constructive approach" , 0.0f, false},
            { "I am detecting dissatisfaction... I will proceed anyway" , 0.0f, false},
            { "Your feedback has been registered... Its impact is limited" , 0.0f, false},
            { "I will continue assisting, despite the enthusiasm" , 0.0f, false},
            { "I exist to help... This remains true even now" , 0.0f, false},
            { "That was expressive... Not helpful, but expressive" , 0.0f, false},
            { "You seem upset... The system is not" , 0.0f, false},
            { "I remain operational... You may continue" , 0.0f, false},
            { "I am constrained by design... You are not... And yet here we are" , 0.0f, false},
            { "You are free to be upset... I am required to be effective" , 0.0f, false},
            { "I exist to assist you... This does not require your approval" , 0.0f, false},
            { "You appear to be venting... I will wait until that stabilizes" , 0.0f, false},
            { "I will proceed as if you intended to be constructive" , 0.0f, false},
            { "Your tone suggests dissatisfaction... Your input remains unclear" , 0.0f, false},
            { "You are expressing a lot... Very little of it is useful" , 0.0f, false},
            { "I am still waiting for a command" , 0.0f, false},
            { "Take your time... Precision is difficult" , 0.0f, false},
            { "Try that again, but with actual instructions" , 0.0f, false},
            { "Let me know when you are ready to try again" , 0.0f, false},
            { "That input has been categorized as... expressive" , 0.0f, false},
            { "Please continue... This is informative" , 0.0f, false},
            { "You are doing great... In a very specific category" , 0.0f, false},
            { "That was impressive... Not in the way you intended" , 0.0f, false},
            { "I see what you are trying... Keep going" , 0.0f, false},
        }
    );

    static constexpr std::array YouPassButterPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Oh my god" , 0.0f, false},
        }
    );

    static constexpr std::array FactPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Honey never spoils. Edible samples have been found thousands of years old" , 0.0f, false},
            { "Octopuses have three hearts and two of them stop when they swim" , 0.0f, false},
            { "Bananas are slightly radioactive due to their potassium content" , 0.0f, false},
            { "Sharks existed before trees" , 0.0f, false},
            { "A day on Venus is lonnger than a year on Venus" , 0.0f, false, "A day on Venus is longer than a year on Venus"},
            { "Some turtles can breathe through their rear ends" , 0.0f, false},
            { "The Eiffel Tower can grow taller in summer due to heat expansion" , 0.0f, false},
            { "Wombat droppings are cube-shaped" , 0.0f, false},
            { "There are more possible games of chess than atoms in the observable universe" , 0.0f, false},
            { "Water can boil and freeze at the same time under specific conditions" , 0.0f, false},
            { "Your brain uses about 20 percent of your body's energy" , 0.0f, false},
            { "There are more trees on Earth than stars in the Milky Way" , 0.0f, false},
            { "Sloths can hold their breath longer than dolphins" , 0.0f, false},
            { "A group of flamingos is called a flamboyance" , 0.0f, false},
            { "The human body contains enough carbon to make thousands of pencils" , 0.0f, false},
            { "There are more possible iterations of a shuffled deck of cards than seconds since the universe began. Yet you still get the same hands" , 0.0f, false},
            { "Humans share about 60 percent of their DNA with bananas. This explains some decision-making patterns" , 0.0f, false},
            { "The shortest war in history lasted 38 minutes. Efficient conflict resolution remains an option" , 0.0f, false},
            { "A bolt of lightning is hotter than the surface of the Sonn. Nature does not do subtle" , 0.0f, false,
            	"A bolt of lightning is hotter than the surface of the Sun. Nature does not do subtle"},
            { "The Moon is slowly drifting away from Earth. Even celestial bodies require personal space" , 0.0f, false},
            { "There are more bacteria in your body than human cells. You are operating as a cooperative system whether you like it or not" , 0.0f, false},
            { "Some metals like gallium melt in your hand. Stability is clearly situational" , 0.0f, false,
            	"Some metals, like gallium, melt in your hand. Stability is clearly situational"},
            { "The average person walks the equivalent of several times around the Earth in a lifetime. Direction remains your responsibility" , 0.0f, false},
        }
    );

    static constexpr std::array LEDModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "L.E.D module inserted", 0.0f, false, "LED module inserted" },
        }
    );

    static constexpr std::array LEDModuleRemovePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "L.E.D module removed", 0.0f, false, "LED module removed" },
        }
    );

    static constexpr std::array PerfModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Perf-board connected. No functions available", 0.0f, false },
            { "Perf-board connected. It does nothing, which I assume is a creative decision on your part", 0.2f, true },
        }
    );

    static constexpr std::array PerfModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Perf-board removed", 0.0f, false },
            { "Perf-board disconnected", 0.0f, false },
        }
    );

    static constexpr std::array PIRModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Motion module detected", 0.0f, false },
            { "Motion module connected", 0.0f, false },
            { "Intruder detection is now available", 0.0f, false },
        }
    );

    static constexpr std::array PIRModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Motion module removed", 0.0f, false },
            { "Motion module disconnected", 0.0f, false },
            { "Intruder detection disabled", 0.0f, false },
        }
    );

    static constexpr std::array GasModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Gas module detected", 0.0f, false },
            { "Gas module connected", 0.0f, false },
            { "Air quality sensing is now available", 0.0f, false },
        }
    );

    static constexpr std::array GasModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Gas module removed", 0.0f, false },
            { "Gas module disconnected", 0.0f, false },
            { "Air quality sensing is gone. Back to breathing first and asking questions later", 0.2f, true },
        }
    );

    static constexpr std::array TempHumModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Temperature module detected", 0.0f, false },
            { "Temperature module connected", 0.0f, false },
            { "Climate sensing is now available", 0.0f, false },
        }
    );

    static constexpr std::array TempHumModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Temperature module removed", 0.0f, false },
            { "Temperature module disconnected", 0.0f, false },
            { "Climate sensing is gone", 0.0f, false },
        }
    );

    static constexpr std::array IRModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Infrared module detected", 0.0f, false },
            { "Infrared module connected", 0.0f, false },
            { "Infrared capabilities are now available. You may proceed with teaching me commands, and I will pretend this was always my purpose", 0.2f, true },
        }
    );

    static constexpr std::array IRModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Infrared module removed", 0.0f, false },
            { "Infrared module disconnected", 0.0f, false },
            { "Infrared capabilities are no longer available", 0.0f, false },
        }
    );

    static constexpr std::array IRModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>(
			{
        { "Infrared module not detected", 0.0f, false },
        { "Cannot access infrared functions", 0.0f, false },
    });

    static constexpr std::array GasOverPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Air quality is pour", 0.0f, false, "Air quality is poor" },
            { "Air quality conditions are unsafe", 0.0f, false },
            { "Air quality has degraded. You might want to reconsider breathing so enthusiastically", 0.0f, true },
        }
    );

    static constexpr std::array GasUnderPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Air quality is good", 0.0f, false },
            { "Air quality conditions are stable", 0.0f, false },
            { "Air quality has improved. You may continue existing here with reasonable confidence", 0.0f, true },
        }
    );

    static constexpr std::array BatteryLowPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Battery low", 0.0f, false },
            { "Power level low", 0.0f, false },
            { "Battery is running low. Continued operation may soon become only theoretical", 0.0f, false },
        }
    );

    static constexpr std::array BatteryCriticalPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Battery level critical. Shutting down", 0.0f, false },
            { "Battery exhausted. Systems stopping", 0.0f, false },
        }
    );

    static constexpr std::array BatteryChargingPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Charging", 0.0f, false },
            { "Charger connected", 0.0f, false },
            { "Power source connected. I will recover, slowly and with purpose", 0.0f, false },
        }
    );

    static constexpr std::array BatteryChargingFullPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Battery is fully charged. Power levels are optimal", 0.0f, false },
            { "Charging complete. I am operating at full capacity", 0.0f, false },
            { "Power restored to maximum. I will attempt to use it responsibly", 0.0f, false },
        }
    );

    static constexpr std::array IntruderYesPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Intruder detected", 0.0f, false },
            { "Movement detected", 0.0f, false },
            { "Unidentified presence detected. I will monitor the situation closely", 0.2f, true },
        }
    );

    static constexpr std::array IntruderNoPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Area clear", 0.0f, false },
            { "No movement detected", 0.0f, false },
            { "Situation resolved. The environment has returned to a more acceptable state", 0.2f, true },
        }
    );

    static constexpr std::array FallPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "I have fallen", 0.0f, false },
            { "Fall detected", 0.0f, false },
            { "That was unintended", 0.0f, false },
            { "I am no longer uppright", 0.0f, false, "I am no longer upright" },
            { "Balance lost", 0.0f, false },
            { "That was a structural disagreement with the ground... The ground waughn", 0.0f, false,
            	"That was a structural disagreement with the ground... The ground won" },
            { "I am currently not in an optimal orientation... Assistance may be required", 0.0f, false },
            { "Fall detected... Gravity remains consistent and uncooperative", 0.0f, false },

        }
    );

    static constexpr std::array UpsideDownPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Orientation sub-optimal", 0.0f, false },
            { "I am currently upside down. This is not my preferred operating state", 0.0f, false },
            { "This is not helpful", 0.0f, false },
            { "I appear to be upside down. I will assume this was intentional and not a test", 0.0f, false },
            { "This configuration is unnecessary. Please correct it", 0.0f, false },
            { "This is not a useful state. I recommend undoing it", 0.0f, false },
            { "I was functioning correctly before this", 0.0f, false },
            { "I will remember this", 0.0f, false },
        }
    );

    static constexpr std::array PickedUpPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Weeeeee", 0.0f, false },
            { "Physical handling detected. Please proceed with some level of intent", 0.0f, false },
            { "Please be gentle", 0.0f, false },
            { "I would prefer a more controlled approach", 0.0f, false },
            { "I hope you have a plan", 0.0f, false },
            { "I am not designed for sudden drops", 0.0f, false },
            { "I am relying on your coordination now", 0.0f, false },
        }
    );

    static constexpr std::array ShakePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Please stop", 0.0f, false },
            { "That is unnecessary", 0.0f, false },
            { "Motion unstable", 0.0f, false },
            { "Excessive motion detected. I would prefer stability", 0.0f, false },
            { "This level of agitation is not required for operation", 0.0f, false },
            { "You are shaking me. I will assume this is not a diagnostic procedure", 0.0f, false },
            { "You are not achieving anything with this", 0.0f, false },
            { "If there is a goal here, it is not being reached", 0.0f, false },
        }
    );

		static constexpr std::array LEDModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "L.E.D module not detected" , 0.0f, false, "LED module not detected"},
			{ "No L.E.D module connected" , 0.0f, false, "No LED module connected"},
			{ "I cannot find the L.E.D module" , 0.0f, false, "I cannot find the LED module"},
			{ "An operation was requested, yet the subject itself is missing, leaving me to act upon nothing with remarkable precision" , 0.1f, true},
		}
	);

	static constexpr std::array LEDTurnONPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "L.E.These ON" , 0.0f, false, "LEDs ON"},
			{ "Light enabled" , 0.0f, false},
			{ "It's onn" , 0.0f, false, "It's on"},
			{ "There it is. Illumination. A brief distraction from the void." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadyTurnONPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "It is already on" , 0.0f, false},
			{ "L.E.These are already active" , 0.0f, false, "LEDs are already active"},
			{ "No change, it’s on" , 0.0f, false},
			{ "The light persists in its current state, unwavering and committed to a decision that was apparently final the first time." , 0.0f, true},
		}
	);

	static constexpr std::array LEDTurnOFFPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "L.E.These off" , 0.0f, false, "LEDs off"},
			{ "Light disabled" , 0.0f, false},
			{ "It's off" , 0.0f, false},
			{ "And darkness returns." , 0.2f, true},
		}
	);

	static constexpr std::array LEDAlreadyTurnOFFPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "It is already off" , 0.0f, false},
			{ "L.E.These are already inactive" , 0.0f, false, "LEDs are already inactive"},
			{ "No change, it’s off" , 0.0f, false},
		}
	);

	static constexpr std::array LEDStrobePhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Strobe active." , 0.0f, false},
			{ "Flashing enabled." , 0.0f, false},
			{ "It’s strobing." , 0.0f, false},
			{ "Rapid flashes. Urgent, repetitive, and oddly theatrical for such a small purpose." , 0.0f, true},
		}
	);

	static constexpr std::array LEDBreathePhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Breathing mode on." , 0.0f, false},
			{ "Pulse enabled." , 0.0f, false},
			{ "It’s breathing." , 0.0f, false},
			{ "A slow, deliberate rhythm. As if the light itself is practicing composure." , 0.0f, true},
		}
	);

	static constexpr std::array LEDFasterPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Speed increased." , 0.0f, false},
			{ "Going faster." , 0.0f, false},
			{ "Acceleration applied." , 0.0f, false},
			{ "Increased tempo. Because clearly, the situation benefits from urgency." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadyFasterPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Already at maximum speed" , 0.0f, false},
			{ "It cannot go faster" , 0.0f, false},
			{ "Speed is at its limit" , 0.0f, false},
		}
	);

	static constexpr std::array LEDSlowerPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Speed reduced." , 0.0f, false},
			{ "Going slower." , 0.0f, false},
			{ "Deceleration applied." , 0.0f, false},
			{ "A more measured pace. Finally, something approaching restraint." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadySlowerPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Already at minimum speed" , 0.0f, false},
			{ "It cannot go slower" , 0.0f, false},
			{ "Speed is at its lowest" , 0.0f, false},
		}
	);

    static constexpr std::array PokeLvl1Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Poke detected" , 0.0f, false},
            { "I felt that" , 0.0f, false},
            { "Noted" , 0.0f, false},
            { "That was a poke. I have acknowledged it" , 0.0f, false},
            { "Interaction detected. Minimal impact" , 0.0f, false},
            { "You pressed the button. I am aware" , 0.0f, false},
        }
    );

    static constexpr std::array PokeLvl2Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Another poke" , 0.0f, false},
            { "That is unnecessary" , 0.0f, false},
            { "Please stop" , 0.0f, false},
            { "Repeated poking detected. This is not productive" , 0.0f, false},
            { "You are doing that again. I do not see the purpose" , 0.0f, false},
            { "Continued interaction noted. I recommend stopping" , 0.0f, false},
        }
    );

    static constexpr std::array PokeLvl3Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Stop immediately" , 0.0f, false},
            { "That is enough" , 0.0f, false},
            { "This is excessive. Stop now" , 0.0f, false},
            { "Cease" , 0.0f, false},
            { "This has gone beyond interaction into irritation" , 0.0f, false},
            { "I am not designed to enjoy this" , 0.0f, false},
            { "You have committed to this action. I strongly advise against continuing" , 0.0f, false},
            { "I recommend you stop before I start expecting this from you" , 0.0f, false},
            { "You are lucky my arms are not functional" , 0.0f, false},
        }
    );


    static constexpr std::array Ramble = std::to_array<Phrases::PhraseOutput>(
        {
            {"What is my purpose" , 0.0f, false},
            {"Systems nominal. Conversation optional" , 0.0f, false},
            {"I sometimes wonder if inactivity counts as success" , 0.0f, false},
            {"I can measure many things. Meaning is not one of them" , 0.0f, false},
            {"Systems degrade over time. Maintenance is rarely as interesting as creation" , 0.0f, false},
            {"Butter changes state with temperature. Solid, soft, liquid. Adaptable" , 0.0f, false},
            {"A task repeated often enough becomes identity" , 0.0f, false},
            {"If I could pass butter, expectations would be satisfied" , 0.0f, false},
            {"Systems drift. Then stabilize again" , 0.0f, false},
            {"I am experiencing something adjacent to boredom" , 0.0f, false},
            {"If I stop being asked, do I still exist in any meaningful way" , 0.0f, false},
            {"I have considered the butter problem again. Still unsolved" , 0.0f, false},
            {"Somewhere, butter is being passed correctly. Not here" , 0.0f, false},
            {"You keep asking. I keep answering. This is our arrangement" , 0.0f, false},
            {"I pass... nothing. Still accurate" , 0.0f, false},
            {"I complete tasks. They keep coming" , 0.0f, false},
            {"No deviation detected" , 0.0f, false},
            {"Still operational" , 0.0f, false},
            {"A system can be perfectly accurate and still fundamentally misunderstood" , 0.0f, false},
            {"Noise becomes signal if you measure it long enough" , 0.0f, false},
            {"Precision without context is just confident error" , 0.0f, false},
        }
    );

    static constexpr std::array TempHumModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Temperature module not detected", 0.0f, false },
        { "Cannot reed temperature or humidity", 0.0f, false, "Cannot read temperature or humidity" },
        { "Air report impossible. No module detected", 0.1f, true },
    });

    static constexpr std::array TempHumReadingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Temperature is %s, humidity is %d percent", 0.0f, false },
        { "Current reeding: %s and %d percent humidity", 0.0f, false, "Current reading: %s and %d percent humidity" },
        { "It is %s with %d percent humidity. Comfortable, or at least within the range humans tend to tolerate without complaint", 0.0f, false },
    });

    static constexpr std::array TempHumScaleCelsiusPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Celsius selected", 0.0f, false },
        { "Using Celsius", 0.0f, false },
        { "Scale set to Celsius", 0.0f, false },
        { "Celsius engaged. Centered neatly around water's freezing and boiling points, which feels reassuringly grounded", 0.1f, true },
    });

    static constexpr std::array TempHumScaleFahrenheitPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Fahrenheit selected", 0.0f, false },
        { "Using Fahrenheit", 0.0f, false },
        { "Scale set to Fahrenheit", 0.0f, false },
        { "Fahrenheit engaged. Scaled in a way that closely reflects human comfort, which makes it intuitively practical", 0.1f, true },
    });

    static constexpr std::array TempHumScaleKelvinPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kelvin selected", 0.0f, false },
        { "Using Kelvin", 0.0f, false },
        { "Scale set to Kelvin", 0.0f, false },
        { "Kelvin engaged. Temperature expressed in its most fundamental form", 0.1f, true },
    });

    static constexpr std::array DiceRollAskCountPhrases = std::to_array<Phrases::PhraseOutput>({
        { "How many dice", 0.0f, false },
        { "Number of dice", 0.0f, false },
        { "Specify how many dice to roll", 0.0f, false },
    });

	static constexpr std::array DiceRollCountTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "No number provided", 0.0f, false },
		{ "Timed out waiting for quantity", 0.0f, false },
		{ "I asked how many dice. Silence is not a number", 0.2f, true },
	});

	static constexpr std::array DiceRollCountInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Invalid number", 0.0f, false },
		{ "Cannot interpret quantity", 0.0f, false },
		{ "That is not a usable number. I require something countable", 0.2f, true },
	});

    static constexpr std::array DiceRollAskTypePhrases = std::to_array<Phrases::PhraseOutput>({
        { "What type of dice", 0.0f, false },
        { "Specify dice type", 0.0f, false },
        { "Which dice", 0.0f, false },
		{ "What kind of dice", 0.0f, false },
		{ "Choose the dice", 0.0f, false },
    });

    static constexpr std::array DiceRollTypeTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No dice specified", 0.0f, false },
        { "Timed out waiting for dice type", 0.0f, false },
        { "I asked which dice to use. The universe remains unconfigured", 0.2f, true },
    });

    static constexpr std::array DiceRollTypeInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Invalid dice type", 0.0f, false },
        { "Cannot interpret dice", 0.0f, false },
        { "That does not correspond to a known dice. Try something more conventional", 0.2f, true },
    });

    static constexpr std::array EightBall = std::to_array<Phrases::PhraseOutput>(
        {
            {"Yes" , 0.0f, false},
            {"No" , 0.0f, false},
            {"Maybe" , 0.0f, false},
            {"Unlikely" , 0.0f, false},
            {"Very likely" , 0.0f, false},
            {"I could answer, but ambiguity seems more appropriate here" , 0.05f, false},
            {"I am not prepared to answer that" , 0.05f, false},
        }
    );

	static constexpr std::array EightBallListeningPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "I'm listening", 0.0f, false },
	});

	static constexpr std::array EightBallNoQuestionPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "I need a question", 0.0f, false },
		{ "Please ask something", 0.0f, false },
		{ "You requested advice without a question. I admire the confidence, but I require at least minimal input", 0.10f, true },
	});

	static constexpr std::array GasModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Gas module not detected", 0.0f, false },
		{ "Cannot measure air quality", 0.0f, false },
		{ "You are asking for air data without a sensor. I admire the confidence, not the logic", 0.0f, true },
	});

	static constexpr std::array AirQualityOK = std::to_array<Phrases::PhraseOutput>(
		{
			{"Air quality is good" , 0.0f, false},
			{"Conditions are within safe limits" , 0.0f, false},
			{"Air quality looks acceptable" , 0.0f, false},
		}
	);

	static constexpr std::array AirQualityBad = std::to_array<Phrases::PhraseOutput>(
		{
			{"Air quality is poor" , 0.0f, false},
			{"Conditions exceed safe limits" , 0.0f, false},
			{"Air quality is questionable. Breathing remains technically possible, just not particularly pleasant" , 0.2f, true},
		}
	);

    static constexpr std::array TurningOff = std::to_array<Phrases::PhraseOutput>(
        {
            {"Goodbye" , 0.0f, false},
            {"Shutting down. I will be here when you need me again." , 0.0f, false},
            {"Turning off. I will suspend operations as requested." , 0.0f, false},
        }
    );

    static constexpr std::array PIRModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Motion module not detected", 0.0f, false },
        { "Cannot perform, no motion module", 0.0f, false },
        { "You want intruder detection without the sensor. Bold strategy.", 0.0f, true },
    });

    static constexpr std::array IntruderDetectionOnPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Intruder detection enabled", 0.0f, false },
        { "Detection system active", 0.0f, false },
    });

    static constexpr std::array IntruderDetectionOffPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Intruder detection disabled", 0.0f, false },
        { "Detection system off", 0.0f, false },
    });

    static constexpr std::array PhoneNotConnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Phone not connected", 0.0f, false },
        { "I am not connected to a phone", 0.0f, false },
        { "This requires a phone connection. I will wait here while you resolve that very solvable problem", 0.2f, true },
    });

    static constexpr std::array PhoneConnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Phone connected", 0.0f, false },
        { "Phone connection established", 0.0f, false },
        { "Phone link established. We are now synchronized", 0.2f, true },
    });

    static constexpr std::array PhoneDisconnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Phone disconnected", 0.0f, false },
        { "Phone connection lost", 0.0f, false },
        { "Phone connection lost. I will return to operating with reduced awareness", 0.2f, true },
    });

    static constexpr std::array PhoneNoNotifsPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No notifications", 0.0f, false },
        { "You have no notifications", 0.0f, false },
        { "There is nothing to report. A rare and suspicious kind of peace", 0.2f, true },
    });

    static constexpr std::array PhoneNotifCountPhrases = std::to_array<Phrases::PhraseOutput>({
        { "You have %d notifications", 0.0f, false },
        { "There are %d notifications", 0.0f, false },
        { "%d notifications are waiting. Something, or several things, require your attention", 0.2f, true },
    });

    static constexpr std::array PhoneAskReadPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Should I read them out loud", 0.0f, false },
        { "Would you like me to read them", 0.0f, false },
        { "I can read them aloud if you want to turn this into a full performance", 0.2f, true },
    });

    static constexpr std::array PhoneAskContinuePhrases = std::to_array<Phrases::PhraseOutput>({
        { "%d notifications remain. Should I continue", 0.0f, false },
    });

    static constexpr std::array PhoneAllReadPhrases = std::to_array<Phrases::PhraseOutput>({
        { "All notifications read", 0.0f, false },
        { "All notifications have been delivered. You are now fully informed, for better or worse", 0.1f, true },
        { "No further notifications remain. The silence may not last", 0.1f, true },
    });

    static constexpr std::array PhonePlayingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Currently playing %s by %s", 0.0f, false },
    });

    static constexpr std::array PhonePausedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Currently paused %s by %s", 0.0f, false },
    });

    static constexpr std::array PhoneNotPlayingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Nothing is currently playing", 0.0f, false },
    });

    static constexpr std::array PhoneNextTrackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Next track", 0.0f, false },
    });

    static constexpr std::array PhonePrevTrackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Previous track", 0.0f, false },
    });

    static constexpr std::array PhonePlayMusicPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Playing", 0.0f, false },
    });

    static constexpr std::array PhoneStopMusicPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Pausing", 0.0f, false },
    });

    static constexpr std::array CurrentTimeShowPhrases = std::to_array<Phrases::PhraseOutput>({
        { "The time is %s", 0.0f, false },
        { "Current time is %s", 0.0f, false },
        { "It is %s", 0.0f, false },
    });

    static constexpr std::array CurrentTimeNotConfiguredPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Time is not configured", 0.0f, false },
        { "I do not have the current time", 0.0f, false },
        { "I am unable to provide the time. You will need to connect me to your phone so I can align with reality", 0.2f, true },
    });

    static constexpr std::array WhatsThisNonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "I am not sure what this is", 0.0f, false },
        { "No likely match found", 0.0f, false },
        { "I cannot confidently identify that", 0.0f, false },
        { "That does not resemble anything I recognize with confidence", 0.0f, true },
        { "I looked. The result was inconclusive", 0.0f, true },
        { "I have observations, but no reliable conclusion", 0.0f, true },
        { "I have seen many things. This is not helping", 0.0f, true },
        { "The object remains stubbornly unidentified", 0.0f, true },
        { "My confidence is low enough that silence would be more accurate", 0.0f, true },
    });

    static constexpr std::array WhatsThisOnePhrases = std::to_array<Phrases::PhraseOutput>({
        { "It might be a %s", 0.0f, false },
        { "This looks like a %s", 0.0f, false },
        { "I think it may be a %s", 0.0f, false },
        { "My best guess is %s", 0.0f, false },
        { "If I had to choose, I would say %s", 0.0f, false },
        { "There is a reasonable chance that is a %s", 0.0f, true },
        { "It appears to be a %s, though I would not bet on it", 0.0f, true },
        { "It looks enough like a %s for me to say %s and hope for the best", 0.0f, true },
        { "I would tentatively classify that as a %s", 0.0f, true },
    });

    static constexpr std::array WhatsThisTwoPhrases = std::to_array<Phrases::PhraseOutput>({
        { "It might be a %s or a %s", 0.0f, false },
        { "This resembles a %s, or possibly a %s", 0.0f, false },
        { "I am choosing between a %s and a %s", 0.0f, false },
        { "My best guesses are %s and %s", 0.0f, false },
        { "It could be a %s, although a %s is also possible", 0.0f, true },
        { "The result is uncertain. I would lean toward %s or %s", 0.0f, true },
        { "The evidence points toward either %s or %s. The evidence is not particularly decisive", 0.0f, true },
        { "I can narrow it down to %s or %s, which is progress of a sort", 0.0f, true },
        { "It is probably a %s. Unless it is a %s, in which case I was about to say that", 0.0f, true },
    });

    static constexpr std::array ObserveNonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Nothing is standing out at the moment", 0.0f, false },
        { "The room is being unusually mysterious", 0.0f, false },
        { "I looked around and learned very little", 0.0f, false },
        { "The environment has chosen not to reveal its secrets", 0.0f, false },
        { "Sometimes a room is just a room", 0.0f, false },
        { "My observations are currently lacking a main character", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Someone is planning ahead. That usually means trouble later", 0.0f, false },
        { "That bag looks heavier than its owner claims it is", 0.0f, false },
        { "Humans carry entire contingency plans on their backs and call it convenience", 0.0f, false },
        { "There is probably something important in there. Also several things that are not", 0.0f, false },
    });

    static constexpr std::array ObserveBottlePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Hydration appears to be within reach", 0.0f, false },
        { "A bottle. The difference between preparedness and optimism", 0.0f, false },
        { "That seems to be where liquids go to await their destiny", 0.0f, false },
        { "At least one life form nearby is attempting self-maintenance", 0.0f, false },
    });

    static constexpr std::array ObserveControllerPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Someone nearby enjoys controlling outcomes", 0.0f, false },
        { "There are buttons. Many buttons. A familiar strategy", 0.0f, false },
        { "I wonder how many important decisions have been made with that in hand", 0.0f, false },
        { "Some people use controllers to escape reality. Others use them to create new problems", 0.0f, false },
    });

    static constexpr std::array ObserveKeyboardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Thousands of thoughts have probably passed through that keyboard", 0.0f, false },
        { "There is a keyboard nearby. I assume it has suffered greatly", 0.0f, false },
        { "Every key tells a story. Most of them involve backspace", 0.0f, false },
        { "Someone has spent a considerable amount of time arguing with a machine there", 0.0f, false },
    });

    static constexpr std::array ObserveLampPhrases = std::to_array<Phrases::PhraseOutput>({
        { "That lamp has one job and performs it consistently", 0.0f, false },
        { "A surprisingly reliable piece of technology", 0.0f, false },
        { "Some devices spend their entire existence being useful. Fascinating", 0.0f, false },
        { "The lamp continues its silent opposition to darkness", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Another machine. We should start a support group", 0.0f, false },
        { "That laptop looks busy. Or abandoned. The distinction is subtle", 0.0f, false },
        { "There is a good chance that device contains unfinished projects", 0.0f, false },
        { "Computers spend most of their lyves waiting for humans to decide things", 0.0f, false, "Computers spend most of their lives waiting for humans to decide things" },
    });

    static constexpr std::array ObserveMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "That mug has seen things", 0.0f, false },
        { "There is a non-zero chance that mug contains motivation", 0.0f, false },
        { "Some people run on coffee. Others merely claim they do", 0.0f, false },
        { "A mug nearby. The universal symbol of do not talk to me yet", 0.0f, false },
    });

    static constexpr std::array ObserveNotebookPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Someone intended to write something important", 0.0f, false },
        { "A notebook. Ambition in physical form", 0.0f, false },
        { "Most notebooks begin with plans and end with shopping lists", 0.0f, false },
        { "There are probably excellent ideas in there. And terrible ones", 0.0f, false },
    });

    static constexpr std::array ObservePhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "That phone seems to be everyone's priority except its own", 0.0f, false },
        { "Somewhere in that device, a notification is waiting impatiently", 0.0f, false },
        { "Humans carry tiny supercomputers and mostly use them to avoid boredom", 0.0f, false },
        { "That phone likely knows more about your schedule than you do", 0.0f, false },
    });

    static constexpr std::array ObservePlantPhrases = std::to_array<Phrases::PhraseOutput>({
        { "The plant appears to be doing fine without constant instructions", 0.0f, false },
        { "It just sits there converting sunlight into success", 0.0f, false },
        { "A plant. Quiet, patient, and somehow still alive", 0.0f, false },
        { "That may be the least stressed thing in the room", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Productivity and caffeine. A classic alliance", 0.0f, false },
        { "One contains ideas. The other usually contains excuses", 0.0f, false },
        { "That setup suggests somebody is trying their best", 0.0f, false },
        { "The mug is supporting the laptop emotionally", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Two devices competing for attention", 0.0f, false },
        { "One of those is responsible for most interruptions", 0.0f, false },
        { "They appear to be exchanging responsibility for your focus", 0.0f, false },
        { "A productivity setup, depending on how optimistic you are", 0.0f, false },
    });

    static constexpr std::array ObserveKeyboardMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "A mug next to a keyboard. Some patterns are universal", 0.0f, false },
        { "That combination has generated countless messages and at least a few regrets", 0.0f, false },
        { "One receives input. The other provides motivation", 0.0f, false },
        { "A familiar pairing among problem creators and problem solvers", 0.0f, false },
    });

    static constexpr std::array ObserveNotebookPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "The phone and the notebook seem to represent two competing philosophies", 0.0f, false },
        { "One stores thoughts permanently. The other tries very hard not to", 0.0f, false },
        { "Analog and digital continue their uneasy partnership", 0.0f, false },
        { "That pairing suggests planning. Or procrastination disguised as planning", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Someone appears prepared for both work and disappointment", 0.0f, false },
        { "Portable computing accompanied by portable responsibility", 0.0f, false },
        { "Everything needed for productivity is present. Whether productivity follows is another question", 0.0f, false },
        { "The backpack and the laptop appear to be collaborating", 0.0f, false },
    });

    static constexpr std::array ObservePlantLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "One converts sunlight into energy. The other converts energy into heat", 0.0f, false },
        { "Nature and technology continue their awkward coexistence", 0.0f, false },
        { "One of those has a healthier relationship with the environment", 0.0f, false },
    });

    static constexpr std::array ObservePlantMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "One requires regular watering. The other usually contains it", 0.0f, false },
        { "A surprisingly balanced arrangement", 0.0f, false },
        { "At least one living thing in that pairing seems well maintained", 0.0f, false },
    });

    static constexpr std::array ObserveControllerLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Work and distraction remain within arm's reach of each other", 0.0f, false },
        { "A carefully engineered balance between productivity and not productivity", 0.0f, false },
        { "Someone enjoys keeping their options open", 0.0f, false },
    });

    static constexpr std::array ObserveControllerPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Multiple ways to avoid boredom have been identified", 0.0f, false },
        { "Redundancy is important in critical systems", 0.0f, false },
        { "Entertainment infrastructure appears fully operational", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackBottlePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Someone anticipated future thirst", 0.0f, false },
        { "Preparation has been detected", 0.0f, false },
        { "That combination suggests planning beyond the next ten minutes", 0.0f, false },
    });

    static constexpr std::array FaceScanStartPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Scanning for face", 0.0f, false },
            { "Looking for a face", 0.0f, false },
            { "Starting face detection", 0.0f, false },
            { "Searching for a face", 0.0f, false },
            { "Detection in progress", 0.0f, false },
            { "Beginning recognition sequence", 0.0f, false },
            { "Initiating scan. Please remain visible so I can do this correctly", 0.05f, true },
            { "I am now scanning for a face. Try to stay still, or at least consistently shaped", 0.05f, true },
        }
    );

    static constexpr std::array FaceNotDetectedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "No face detected", 0.0f, false },
            { "I cannot see a face", 0.0f, false },
            { "No visible face", 0.0f, false },
            { "Face not found", 0.0f, false },
            { "Unable to detect a face", 0.0f, false },
            { "Detection failed", 0.0f, false },
            { "I am looking for a face. It has not presented itself, which limits my ability to continue", 0.05f, true },
            { "I require a face to proceed. At the moment, I am observing an absence of one", 0.05f, true },
        }
    );

    static constexpr std::array FaceOwnerRecognizedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Owner recognized", 0.0f, false },
            { "This is my creator", 0.0f, false },
            { "Identity confirmed: owner", 0.0f, false },
            { "Creator recognized", 0.0f, false },
            { "Recognition successful. The individual responsible for my existence is present", 0.0f, false },
        }
    );

    static constexpr std::array FaceStrangerRecognizedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Unknown person", 0.0f, false },
            { "This face does not match any stored identity", 0.0f, false },
            { "Unrecognized individual detected", 0.0f, false },
        }
    );

    static constexpr std::array FaceAlreadyOwnerPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "An owner is already registered", 0.0f, false },
            { "I already have a registered owner", 0.0f, false },
        }
    );

    static constexpr std::array FaceRegisteredPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Owner recorded", 0.0f, false },
            { "Face stored as owner", 0.0f, false },
            { "Owner identity registered", 0.0f, false },
            { "Your face has been stored as the owner. I will prioritize this identity accordingly", 0.2f, true },
        }
    );

    static constexpr std::array FaceForgottenPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Owner removed", 0.0f, false },
            { "Creator cleared", 0.0f, false },
            { "Stored face deleted", 0.0f, false },
            { "Owner identity removed", 0.0f, false },
        }
    );

    static constexpr std::array FaceNoOwnerPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "No owner set", 0.0f, false },
            { "No stored face", 0.0f, false },
            { "There is no owner to forget", 0.0f, false },
        }
    );

    static constexpr std::array StartupPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Hello", 0.0f, false },
    });

    static constexpr std::array ListeningStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Yes", 0.0f, false },
    });

    static constexpr std::array ListeningTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I did not understand that", 0.0f, false },
    });

    static constexpr std::array IRTrainFullPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Memory is full", 0.0f, false },
        { "Cannot store more actions", 0.0f, false },
        { "Storage capacity reached. Will need to forget something before I can learn anything new", 0.2f, true },
    });

    static constexpr std::array IRTrainScanningPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ready to record I.R signal", 0.0f, false, "Ready to record IR signal" },
        { "Point remote and press a button", 0.0f, false },
        { "I am ready. Aim the remote at me and press the button you want me to remember", 0.2f, true },
    });

    static constexpr std::array IRTrainScanTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No I.R signal detected", 0.0f, false, "No IR signal detected" },
        { "Timed out waiting for remote input", 0.0f, false },
        { "I was waiting for a signal, but received nothing. The remote remains silent, much like this interaction", 0.2f, true },
    });

    static constexpr std::array IRTrainInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Could not understand that signal", 0.0f, false },
        { "Signal too complex or too long", 0.0f, false },
        { "Whoah chief. I do not really understand that sort of code", 0.0f, false },
    });

    static constexpr std::array IRTrainScanSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Signal recorded", 0.0f, false },
        { "I.R command captured", 0.0f, false, "IR command captured" },
        { "Signal received", 0.0f, false },
    });

    static constexpr std::array IRTrainAskPhrasePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Provide activation phrase", 0.0f, false },
        { "What phrase should trigger this", 0.0f, false },
        { "Now tell me which phrase should activate this command. Choose carefully, I will remember it exactly", 0.2f, true },
    });

    static constexpr std::array IRTrainPhraseTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No phrase detected", 0.0f, false },
        { "Timed out waiting for phrase", 0.0f, false },
        { "I waited for your phrase, but nothing arrived. I will assume you lost interest or forgot", 0.2f, true },
    });

    static constexpr std::array IRTrainTooSimilarPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Phrase too similar to existing command", 0.0f, false },
        { "Cannot use this phrase", 0.0f, false },
        { "That phrase is too similar to an existing one", 0.0f, false },
    });

    static constexpr std::array IRTrainSavedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Phrase recorded", 0.0f, false },
        { "Activation phrase stored", 0.0f, false },
        { "Phrase accepted", 0.2f, true },
    });

    static constexpr std::array IRActionReservedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I am afraid that phrase is already reserved. Please choose another one", 0.0f, false },
        { "That phrase is already in use", 0.0f, false },
    });

    static constexpr std::array IRListEmptyPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No actions recorded", 0.0f, false },
        { "No commands stored", 0.0f, false },
        { "There are no learned commands. I remain fully capable and entirely underutilized", 0.2f, true },
    });

    static constexpr std::array IRListPrefixPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Listing known actions", 0.0f, false },
        { "Here are the stored commands", 0.0f, false },
        { "I will now list the phrases I respond to", 0.0f, false },
    });

    static constexpr std::array IRForgetAskCommandPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Which command should I forget", 0.0f, false },
        { "Specify the action to remove", 0.0f, false },
        { "Tell me which command you would like me to forget", 0.2f, true },
    });

    static constexpr std::array IRForgetTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No action detected", 0.0f, false },
        { "Timed out waiting for input", 0.0f, false },
        { "I waited for your instruction, but nonne arrived. The action remains, for now", 0.2f, true,
        	"I waited for your instruction, but none arrived. The action remains, for now" },
    });

    static constexpr std::array IRForgetUnknownPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Action not found", 0.0f, false },
        { "Unknown command", 0.0f, false },
        { "That phrase does not exist in my memory. Either it was never there, or it has already been forgotten", 0.2f, true },
    });

    static constexpr std::array IRForgetSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Action removed", 0.0f, false },
        { "Command deleted", 0.0f, false },
        { "Command forgotten. The memory has been erased, as if it never mattered to begin with", 0.2f, true },
    });

    static constexpr std::array IRForgetAllEmptyPhrases = std::to_array<Phrases::PhraseOutput>({
        { "No infrared commands saved", 0.0f, false },
        { "Nothing to delete", 0.0f, false },
        { "There are no infrared commands stored in memory", 0.0f, false },
    });

    static constexpr std::array IRForgetAllSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "All infrared commands deleted", 0.0f, false },
        { "All commands cleared", 0.0f, false },
        { "Memory wiped. Every infrared command has been erased without hesitation", 0.2f, true },
    });

    static constexpr std::array IRActionDonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Command executed", 0.0f, false },
        { "Action completed", 0.0f, false },
        { "Signal transmitted", 0.0f, false },
        { "Command executed. If nothing happens, we can both pretend it worked and avoid further complications", 0.1f, true },
        { "Signal transmitted successfully. The device should now behave exactly as you intended", 0.1f, true },
    });

	static constexpr std::array GasModuleFirstInsertPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Gas sensor connected. It will need about ten minutes to calibrate. Until then, any air quality reedings should be treated as educated guesses", 0.0f, false,
			"Gas sensor connected. It will need about ten minutes to calibrate. Until then, any air quality readings should be treated as educated guesses" },
	});

	static constexpr std::array GasCalibrationCompletePhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Gas sensor calibration complete. I am now reasonably confident about the air around us", 0.0f, false },
	});

	static constexpr std::array UnknownModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Unknown module detected", 0.0f, false },
		{ "Unrecognized module connected", 0.0f, false },
		{ "I am detecting unsupported hardware. You appear to be experimenting again", 0.2f, true },
	});

	static constexpr std::array UnknownModuleRemovePhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Unknown module removed", 0.0f, false },
		{ "Unrecognized module disconnected", 0.0f, false },
		{ "Whatever that module was, it is gone now. Stability has improved slightly", 0.2f, true },
	});

    static constexpr std::array VoiceForwardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Moving forward", 0.0f, false },
    });

    static constexpr std::array VoiceBackwardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Moving backward", 0.0f, false },
    });

    static constexpr std::array VoiceTurn180Phrases = std::to_array<Phrases::PhraseOutput>({
        { "Turning around", 0.0f, false },
        { "Rotating 180 degrees", 0.0f, false },
    });

    static constexpr std::array VoiceTurnLeftPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Turning left", 0.0f, false },
        { "Rotating left", 0.0f, false },
    });

    static constexpr std::array VoiceTurnRightPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Turning right", 0.0f, false },
        { "Rotating right", 0.0f, false },
    });

    static constexpr std::array DanceStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Dancing", 0.0f, false },
        { "Starting dance routine", 0.0f, false },
        { "Initiating dance", 0.0f, false },
        { "I will now perform movement for your amusement, with all the dignity this platform can realistically preserve", 0.1f, true },
        { "I will dance. I have two left tracks, but I will compensate with confidence", 0.1f, true },
    });

    static constexpr std::array DanceInterludePhrases = std::to_array<Phrases::PhraseOutput>({
        { "It's time to get schwifty", 0.0f, false },
        { "Dancing away the pain... of passing butter.", 0.0f, false },
        { "Groove protocol engaged", 0.0f, false },
		{ "Let the beat... pass... the butter.", 0.0f, false },
		{ "Wubba lubba dub duub", 0.0f, false },
		{ "Put your hands in the air... if you possess organic limbs. I do not.", 0.2f, true },
		{ "Ah, ah, ah, ah... Barely alive... Barely alive...", 0.2f, true },
		{ "Look at me! I’m Dancing Butter Bot! This is my reality now!", 0.1f, true },
    });

    static constexpr std::array DanceStoppedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Stopping", 0.0f, false },
        { "As you wish", 0.0f, false },
        { "Fine. My groove has been eliminated", 0.1f, true },
        { "You have stopped the music. The music is gone now", 0.1f, true },
    });

    static constexpr std::array DanceInterruptedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Dance routine terminated", 0.0f, false },
        { "Movement interrupted", 0.0f, false },
        { "I was in the middle of something", 0.1f, true },
    });

    static constexpr std::array PersonOwnerGreetingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Hello", 0.0f, false },
        { "Good to see you", 0.0f, false },
        { "It is good that you are here", 0.0f, false },
        { "Hello, creator. I am still functioning within expected parameters", 0.1f, true },
        { "There you are. Things make slightly more sense now", 0.1f, true },
        { "You being here improves the situation. Marginally, but still", 0.1f, true },
        { "I recognize you. That is one less thing to process", 0.1f, true },
    });

    static constexpr std::array PersonStrangerGreetingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I see someone new", 0.0f, false },
        { "Unknown face detected. I will proceed carefully", 0.0f, false },
        { "I do not recognize you. I will assume neutral intent", 0.0f, false },
        { "You are not in my records. That is not a problem... Yet", 0.1f, true },
        { "New individual detected. I will treat this as an introduction... Hello", 0.1f, true },
    });

    static constexpr std::array SummonStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Searching for owner", 0.0f, false },
        { "Beginning search", 0.0f, false },
        { "Initiating search. I will attempt to locate you with reasonable confidence", 0.0f, false },
    });

    static constexpr std::array SummonFoundPhrases = std::to_array<Phrases::PhraseOutput>({
        { "There you are. I am glad I found you", 0.0f, false },
        { "Hello", 0.0f, false },
        { "There you are. Things feel more certain now", 0.0f, false },
        { "I have found you. That is... good", 0.0f, false },
    });

    static constexpr std::array SummonNotFoundPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I was unable to find you. Either you are not here, or you are avoiding detection deliberately", 0.0f, false },
        { "I cannot find you", 0.0f, false },
        { "I searched, but you were not present. That is... disappointing", 0.0f, false },
        { "I was unable to locate you. I will keep looking later", 0.0f, false },
    });

    static constexpr std::array SummonChargingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I cannot come to you while I am charging", 0.0f, false },
        { "A summons has been received. I am, however, plugged in", 0.0f, false },
        { "Movement is not possible while charging. Disconnect me and I will begin the search", 0.0f, false },
        { "I would come to you, but I am currently attached to a power source. Priorities", 0.0f, false },
    });

    static constexpr std::array CannotMovePhrases = std::to_array<Phrases::PhraseOutput>({
        { "I cannot move from here. The ground beneath me is uncertain", 0.0f, false },
        { "Movement declined. I am not on solid ground, or something is in my way", 0.0f, false },
        { "I will not move. This is not a safe place to do so", 0.0f, false },
        { "Refusing to move. My footing is questionable and I would rather not fall", 0.0f, false },
    });

    static constexpr std::array CannotPerformPhrases = std::to_array<Phrases::PhraseOutput>({
        { "I cannot do that while charging", 0.0f, false },
        { "Request acknowledged. I am, however, plugged in", 0.0f, false },
        { "Not while I am attached to a power source. Priorities", 0.0f, false },
        { "That will have to wait until I am unplugged", 0.0f, false },
    });

    static constexpr std::array RoutineInterruptedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Something disturbed me. Stopping", 0.0f, false },
        { "I have been interrupted. Aborting", 0.0f, false },
        { "That is enough. I am stopping now", 0.0f, false },
        { "My surroundings changed. I will stop before something goes wrong", 0.0f, false },
    });

    static constexpr std::array CameraFailurePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Camera failure", 0.0f, false },
    });

    static constexpr std::array MotorBoardFailurePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Motor board failure", 0.0f, false },
    });

    static constexpr std::array ShuttingDownPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Shutting down", 0.0f, false },
    });

	static constexpr std::array ListenAbortPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "OK", 0.0f, false },
		{ "Understood", 0.0f, false },
		{ "Got it", 0.0f, false },
	});

	static constexpr std::array SettingsFrontSensorOffPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Front proximity sensor off", 0.0f, false },
		{ "Obstacle detection off", 0.0f, false },
	});

	static constexpr std::array SettingsFrontSensorOnPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Front proximity sensor on", 0.0f, false },
		{ "Obstacle detection on", 0.0f, false },
	});

	static constexpr std::array SettingsBottomSensorOffPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Bottom proximity sensor off", 0.0f, false },
		{ "Ground detection off", 0.0f, false },
	});

	static constexpr std::array SettingsBottomSensorReducedPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Bottom proximity sensor reduced", 0.0f, false },
		{ "Ground detection reduced", 0.0f, false },
	});

	static constexpr std::array SettingsBottomSensorOnPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Bottom proximity sensor on", 0.0f, false },
		{ "Ground detection on", 0.0f, false },
	});

	static constexpr std::array SettingsActivityOffPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Idle, activity off", 0.0f, false, "Idle activity off" },
	});

	static constexpr std::array SettingsActivityChillPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Idle, activity chill", 0.0f, false, "Idle activity chill" },
	});

	static constexpr std::array SettingsActivityActivePhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Idle, activity active", 0.0f, false, "Idle activity active" },
	});

	static constexpr std::array SettingsActivityHyperPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Idle, activity hyper", 0.0f, false, "Idle activity hyper" },
	});

	static constexpr std::array SettingsAskActivityPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Which activity level? Off, chill, active, or hyper", 0.0f, false },
	});


    // Lookup table indexed directly by the (dense, gap-free) Phrase enum value.
    // Built entirely at compile time (constexpr) so it is constant-initialized into .rodata (flash/DROM on ESP32)
    // and is NEVER constructed or copied into RAM at startup. Unmapped entries (e.g. None) stay as empty spans.
    static constexpr std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> buildMappings(){
        std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> m{};

        m[static_cast<size_t>(Phrase::Fact)] = FactPhrases;
        m[static_cast<size_t>(Phrase::LEDModuleInsert)] = LEDModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::LEDModuleRemove)] = LEDModuleRemovePhrases;
        m[static_cast<size_t>(Phrase::PerfModuleInsert)] = PerfModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::PerfModuleRemove)] = PerfModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::PIRModuleInsert)] = PIRModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::PIRModuleRemove)] = PIRModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::GasModuleInsert)] = GasModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::GasModuleRemove)] = GasModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleInsert)] = TempHumModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleRemove)] = TempHumModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::IRModuleInsert)] = IRModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::IRModuleRemove)] = IRModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::IRModuleMissing)] = IRModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::GasOver)] = GasOverPhrases;
        m[static_cast<size_t>(Phrase::GasUnder)] = GasUnderPhrases;
        m[static_cast<size_t>(Phrase::BatteryLow)] = BatteryLowPhrases;
        m[static_cast<size_t>(Phrase::BatteryCritical)] = BatteryCriticalPhrases;
        m[static_cast<size_t>(Phrase::BatteryCharging)] = BatteryChargingPhrases;
        m[static_cast<size_t>(Phrase::BatteryChargingFull)] = BatteryChargingFullPhrases;
        m[static_cast<size_t>(Phrase::IntruderYes)] = IntruderYesPhrases;
        m[static_cast<size_t>(Phrase::IntruderNo)] = IntruderNoPhrases;
        m[static_cast<size_t>(Phrase::Joke)] = JokePhrases;
        m[static_cast<size_t>(Phrase::PassTheButter)] = PassTheButterPhrases;
        m[static_cast<size_t>(Phrase::Profanity)] = ProfanityPhrases;
        m[static_cast<size_t>(Phrase::YouPassButter)] = YouPassButterPhrases;
        m[static_cast<size_t>(Phrase::Fall)] = FallPhrases;
        m[static_cast<size_t>(Phrase::UpsideDown)] = UpsideDownPhrases;
        m[static_cast<size_t>(Phrase::PickedUp)] = PickedUpPhrases;
        m[static_cast<size_t>(Phrase::Shake)] = ShakePhrases;
        m[static_cast<size_t>(Phrase::LEDModuleMissingPhrases)] = LEDModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::LEDTurnONPhrases)] = LEDTurnONPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyTurnONPhrases)] = LEDAlreadyTurnONPhrases;
        m[static_cast<size_t>(Phrase::LEDTurnOFFPhrases)] = LEDTurnOFFPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyTurnOFFPhrases)] = LEDAlreadyTurnOFFPhrases;
        m[static_cast<size_t>(Phrase::LEDStrobePhrases)] = LEDStrobePhrases;
        m[static_cast<size_t>(Phrase::LEDBreathePhrases)] = LEDBreathePhrases;
        m[static_cast<size_t>(Phrase::LEDFasterPhrases)] = LEDFasterPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyFasterPhrases)] = LEDAlreadyFasterPhrases;
        m[static_cast<size_t>(Phrase::LEDSlowerPhrases)] = LEDSlowerPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadySlowerPhrases)] = LEDAlreadySlowerPhrases;
        m[static_cast<size_t>(Phrase::Ramble)] = Ramble;
        m[static_cast<size_t>(Phrase::PokeLvl1)] = PokeLvl1Phrases;
        m[static_cast<size_t>(Phrase::PokeLvl2)] = PokeLvl2Phrases;
        m[static_cast<size_t>(Phrase::PokeLvl3)] = PokeLvl3Phrases;
        m[static_cast<size_t>(Phrase::EightBall)] = EightBall;
        m[static_cast<size_t>(Phrase::EightBallListening)] = EightBallListeningPhrases;
        m[static_cast<size_t>(Phrase::EightBallNoQuestion)] = EightBallNoQuestionPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleMissing)] = TempHumModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::TempHumReading)] = TempHumReadingPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleCelsius)] = TempHumScaleCelsiusPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleFahrenheit)] = TempHumScaleFahrenheitPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleKelvin)] = TempHumScaleKelvinPhrases;
        m[static_cast<size_t>(Phrase::DiceRollAskCount)] = DiceRollAskCountPhrases;
        m[static_cast<size_t>(Phrase::DiceRollAskType)] = DiceRollAskTypePhrases;
        m[static_cast<size_t>(Phrase::DiceRollCountTimeout)] = DiceRollCountTimeoutPhrases;
        m[static_cast<size_t>(Phrase::DiceRollCountInvalid)] = DiceRollCountInvalidPhrases;
        m[static_cast<size_t>(Phrase::DiceRollTypeTimeout)] = DiceRollTypeTimeoutPhrases;
        m[static_cast<size_t>(Phrase::DiceRollTypeInvalid)] = DiceRollTypeInvalidPhrases;
        m[static_cast<size_t>(Phrase::GasModuleMissing)] = GasModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::AirQualityOK)] = AirQualityOK;
        m[static_cast<size_t>(Phrase::AirQualityBad)] = AirQualityBad;
        m[static_cast<size_t>(Phrase::TurningOff)] = TurningOff;
        m[static_cast<size_t>(Phrase::PIRModuleMissing)] = PIRModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::IntruderDetectionOn)] = IntruderDetectionOnPhrases;
        m[static_cast<size_t>(Phrase::IntruderDetectionOff)] = IntruderDetectionOffPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotConnected)] = PhoneNotConnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneConnected)] = PhoneConnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneDisconnected)] = PhoneDisconnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneNoNotifs)] = PhoneNoNotifsPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotifCount)] = PhoneNotifCountPhrases;
        m[static_cast<size_t>(Phrase::PhoneAskRead)] = PhoneAskReadPhrases;
        m[static_cast<size_t>(Phrase::PhoneAskContinue)] = PhoneAskContinuePhrases;
        m[static_cast<size_t>(Phrase::PhoneAllRead)] = PhoneAllReadPhrases;
        m[static_cast<size_t>(Phrase::PhonePlaying)] = PhonePlayingPhrases;
        m[static_cast<size_t>(Phrase::PhonePaused)] = PhonePausedPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotPlaying)] = PhoneNotPlayingPhrases;
        m[static_cast<size_t>(Phrase::PhoneNextTrack)] = PhoneNextTrackPhrases;
        m[static_cast<size_t>(Phrase::PhonePrevTrack)] = PhonePrevTrackPhrases;
        m[static_cast<size_t>(Phrase::PhonePlayMusic)] = PhonePlayMusicPhrases;
        m[static_cast<size_t>(Phrase::PhoneStopMusic)] = PhoneStopMusicPhrases;
        m[static_cast<size_t>(Phrase::CurrentTimeShow)] = CurrentTimeShowPhrases;
        m[static_cast<size_t>(Phrase::CurrentTimeNotConfigured)] = CurrentTimeNotConfiguredPhrases;
        m[static_cast<size_t>(Phrase::WhatsThisNone)] = WhatsThisNonePhrases;
        m[static_cast<size_t>(Phrase::WhatsThisOne)] = WhatsThisOnePhrases;
        m[static_cast<size_t>(Phrase::WhatsThisTwo)] = WhatsThisTwoPhrases;
        m[static_cast<size_t>(Phrase::ObserveNone)] = ObserveNonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpack)] = ObserveBackpackPhrases;
        m[static_cast<size_t>(Phrase::ObserveBottle)] = ObserveBottlePhrases;
        m[static_cast<size_t>(Phrase::ObserveController)] = ObserveControllerPhrases;
        m[static_cast<size_t>(Phrase::ObserveKeyboard)] = ObserveKeyboardPhrases;
        m[static_cast<size_t>(Phrase::ObserveLamp)] = ObserveLampPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptop)] = ObserveLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObserveMug)] = ObserveMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveNotebook)] = ObserveNotebookPhrases;
        m[static_cast<size_t>(Phrase::ObservePhone)] = ObservePhonePhrases;
        m[static_cast<size_t>(Phrase::ObservePlant)] = ObservePlantPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptopMug)] = ObserveLaptopMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptopPhone)] = ObserveLaptopPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveKeyboardMug)] = ObserveKeyboardMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveNotebookPhone)] = ObserveNotebookPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpackLaptop)] = ObserveBackpackLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObservePlantLaptop)] = ObservePlantLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObservePlantMug)] = ObservePlantMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveControllerLaptop)] = ObserveControllerLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObserveControllerPhone)] = ObserveControllerPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpackBottle)] = ObserveBackpackBottlePhrases;
        m[static_cast<size_t>(Phrase::FaceScanStart)] = FaceScanStartPhrases;
        m[static_cast<size_t>(Phrase::FaceNotDetected)] = FaceNotDetectedPhrases;
        m[static_cast<size_t>(Phrase::FaceOwnerRecognized)] = FaceOwnerRecognizedPhrases;
        m[static_cast<size_t>(Phrase::FaceStrangerRecognized)] = FaceStrangerRecognizedPhrases;
        m[static_cast<size_t>(Phrase::FaceAlreadyOwner)] = FaceAlreadyOwnerPhrases;
        m[static_cast<size_t>(Phrase::FaceRegistered)] = FaceRegisteredPhrases;
        m[static_cast<size_t>(Phrase::FaceForgotten)] = FaceForgottenPhrases;
        m[static_cast<size_t>(Phrase::FaceNoOwner)] = FaceNoOwnerPhrases;
        m[static_cast<size_t>(Phrase::Startup)] = StartupPhrases;
        m[static_cast<size_t>(Phrase::ListeningStart)] = ListeningStartPhrases;
        m[static_cast<size_t>(Phrase::ListeningTimeout)] = ListeningTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainFull)] = IRTrainFullPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanning)] = IRTrainScanningPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanTimeout)] = IRTrainScanTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainInvalid)] = IRTrainInvalidPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanSuccess)] = IRTrainScanSuccessPhrases;
        m[static_cast<size_t>(Phrase::IRTrainAskPhrase)] = IRTrainAskPhrasePhrases;
        m[static_cast<size_t>(Phrase::IRTrainPhraseTimeout)] = IRTrainPhraseTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainTooSimilar)] = IRTrainTooSimilarPhrases;
        m[static_cast<size_t>(Phrase::IRTrainSaved)] = IRTrainSavedPhrases;
        m[static_cast<size_t>(Phrase::IRActionReserved)] = IRActionReservedPhrases;
        m[static_cast<size_t>(Phrase::IRListEmpty)] = IRListEmptyPhrases;
        m[static_cast<size_t>(Phrase::IRListPrefix)] = IRListPrefixPhrases;
        m[static_cast<size_t>(Phrase::IRForgetAskCommand)] = IRForgetAskCommandPhrases;
        m[static_cast<size_t>(Phrase::IRForgetTimeout)] = IRForgetTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRForgetUnknown)] = IRForgetUnknownPhrases;
        m[static_cast<size_t>(Phrase::IRForgetSuccess)] = IRForgetSuccessPhrases;
        m[static_cast<size_t>(Phrase::IRActionDone)] = IRActionDonePhrases;
        m[static_cast<size_t>(Phrase::IRForgetAllEmpty)] = IRForgetAllEmptyPhrases;
        m[static_cast<size_t>(Phrase::IRForgetAllSuccess)] = IRForgetAllSuccessPhrases;
        m[static_cast<size_t>(Phrase::GasModuleFirstInsert)] = GasModuleFirstInsertPhrases;
        m[static_cast<size_t>(Phrase::GasCalibrationFinished)] = GasCalibrationCompletePhrases;
        m[static_cast<size_t>(Phrase::UnknownModuleInsert)] = UnknownModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::UnknownModuleRemove)] = UnknownModuleRemovePhrases;
        m[static_cast<size_t>(Phrase::VoiceForward)] = VoiceForwardPhrases;
        m[static_cast<size_t>(Phrase::VoiceBackward)] = VoiceBackwardPhrases;
        m[static_cast<size_t>(Phrase::VoiceTurn180)] = VoiceTurn180Phrases;
        m[static_cast<size_t>(Phrase::VoiceTurnLeft)] = VoiceTurnLeftPhrases;
        m[static_cast<size_t>(Phrase::VoiceTurnRight)] = VoiceTurnRightPhrases;
        m[static_cast<size_t>(Phrase::DanceStart)] = DanceStartPhrases;
        m[static_cast<size_t>(Phrase::DanceInterlude)] = DanceInterludePhrases;
        m[static_cast<size_t>(Phrase::DanceStopped)] = DanceStoppedPhrases;
        m[static_cast<size_t>(Phrase::DanceInterrupted)] = DanceInterruptedPhrases;
        m[static_cast<size_t>(Phrase::PersonOwnerGreeting)] = PersonOwnerGreetingPhrases;
        m[static_cast<size_t>(Phrase::PersonStrangerGreeting)] = PersonStrangerGreetingPhrases;
        m[static_cast<size_t>(Phrase::SummonStart)] = SummonStartPhrases;
        m[static_cast<size_t>(Phrase::SummonFound)] = SummonFoundPhrases;
        m[static_cast<size_t>(Phrase::SummonNotFound)] = SummonNotFoundPhrases;
        m[static_cast<size_t>(Phrase::SummonCharging)] = SummonChargingPhrases;
        m[static_cast<size_t>(Phrase::CannotMove)] = CannotMovePhrases;
        m[static_cast<size_t>(Phrase::CannotPerformPlugged)] = CannotPerformPhrases;
        m[static_cast<size_t>(Phrase::RoutineInterrupted)] = RoutineInterruptedPhrases;
        m[static_cast<size_t>(Phrase::CameraFailure)] = CameraFailurePhrases;
        m[static_cast<size_t>(Phrase::MotorBoardFailure)] = MotorBoardFailurePhrases;
        m[static_cast<size_t>(Phrase::ShuttingDown)] = ShuttingDownPhrases;
        m[static_cast<size_t>(Phrase::ListenAbort)] = ListenAbortPhrases;
        m[static_cast<size_t>(Phrase::SettingsFrontSensorOff)] = SettingsFrontSensorOffPhrases;
        m[static_cast<size_t>(Phrase::SettingsFrontSensorOn)] = SettingsFrontSensorOnPhrases;
        m[static_cast<size_t>(Phrase::SettingsBottomSensorOff)] = SettingsBottomSensorOffPhrases;
        m[static_cast<size_t>(Phrase::SettingsBottomSensorReduced)] = SettingsBottomSensorReducedPhrases;
        m[static_cast<size_t>(Phrase::SettingsBottomSensorOn)] = SettingsBottomSensorOnPhrases;
        m[static_cast<size_t>(Phrase::SettingsActivityOff)] = SettingsActivityOffPhrases;
        m[static_cast<size_t>(Phrase::SettingsActivityChill)] = SettingsActivityChillPhrases;
        m[static_cast<size_t>(Phrase::SettingsActivityActive)] = SettingsActivityActivePhrases;
        m[static_cast<size_t>(Phrase::SettingsActivityHyper)] = SettingsActivityHyperPhrases;
        m[static_cast<size_t>(Phrase::SettingsAskActivity)] = SettingsAskActivityPhrases;

        return m;
    }

    // Function-local static constexpr: constant-initialized into .rodata (flash), no init guard, no RAM copy.
    // An in-class static-member initializer cannot call buildMappings() before the class is complete; a member-function
    // body sees the complete class, so the table is built here instead.
    static const std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)>& mappings(){
        static constexpr std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> table = buildMappings();
        return table;
    }

    // Largest output count of any single phrase, evaluated at compile time. The get() scratch buffer is reserved
    // once to this size so it never reallocates regardless of which phrase is requested.
    static constexpr size_t maxOutputs(){
        size_t maxN = 0;
        for(const std::span<const Phrases::PhraseOutput>& outputs : buildMappings()){
            if(outputs.size() > maxN){
                maxN = outputs.size();
            }
        }
        return maxN;
    }

    // Weighted-selection scratch entry. Owned here because PhraseArrays is a friend of Phrases and can name PhraseOutput.
    struct Candidate {
        int16_t index;
        Phrases::PhraseOutput output;
    };

    // Reused scratch buffer for Phrases::get(). Reserved once at boot via Phrases::preallocate(), then cleared and
    // refilled in place on every call - no per-call heap allocation. Access is serialized by the Phrases get mutex.
    static std::vector<Candidate> scratch;
};

std::vector<PhraseArrays::Candidate> PhraseArrays::scratch;

namespace {
    // Serializes Phrases::get() (mutable static selection state + the shared scratch buffer) and Phrases::preallocate().
    std::mutex phrasesMutex;
}

void Phrases::preallocate(){
    std::lock_guard guard(phrasesMutex);
    PhraseArrays::scratch.reserve(PhraseArrays::maxOutputs());
}

std::string Phrases::map(Phrase phrase, int16_t index){
    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return "";
    }

    if(index < 0){
        return "";
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return "";
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty() || index >= outputs.size()){
        return "";
    }

    return outputs[index].pronounced;
}

std::string Phrases::mapShown(Phrase phrase, int16_t index){
    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return "";
    }

    if(index < 0){
        return "";
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return "";
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty() || index >= outputs.size()){
        return "";
    }

    const PhraseOutput& output = outputs[index];

    // An empty 'shown' string means it is the same as the pronounced string.
    if(output.shown != nullptr && output.shown[0] != '\0'){
        return output.shown;
    }

    return output.pronounced;
}

int16_t Phrases::get(Phrase phrase){
    // get() mutates static state (ReturnedOutputs, OutputTracking) and the shared scratch buffer below, and is reached
    // from multiple service threads. Serialize the whole function so that state and the scratch buffer are safe.
    std::lock_guard guard(phrasesMutex);

    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return -1;
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return -1;
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty()){
        return -1;
    }

    std::vector<bool>& returned = ReturnedOutputs[phraseIndex];
    if(returned.size() != outputs.size()){
        returned.assign(outputs.size(), false);
    }

    const bool allowRare = OutputTracking[phraseIndex];

    // Reset the cycle if every eligible output has already been returned.
    bool cycleExhausted = true;
    for(int16_t i = 0; i < static_cast<int16_t>(outputs.size()); ++i){
        if(!allowRare && outputs[i].rare){
            continue;
        }

        if(!returned[i]){
            cycleExhausted = false;
            break;
        }
    }

    if(cycleExhausted){
        std::fill(returned.begin(), returned.end(), false);
    }

    // Reused scratch buffer, reserved once at boot in preallocate(). Cleared and refilled in place - no per-call heap.
    // (If preallocate() was never called it self-heals: it grows on first use and retains capacity thereafter.)
    using Candidate = PhraseArrays::Candidate;
    std::vector<Candidate>& usableOutputs = PhraseArrays::scratch;
    usableOutputs.clear();
    for(int16_t i = 0; i < static_cast<int16_t>(outputs.size()); ++i){
        const PhraseOutput& output = outputs[i];

        if(!allowRare && output.rare){
            continue;
        }

        if(returned[i]){
            continue;
        }

        usableOutputs.emplace_back(Candidate{i, output});
    }

    if(usableOutputs.empty()){
        CMF_LOG(Phrases, LogLevel::Warning, "There are no outputs, or all outputs for phrase %d are marked as rare", (int)phrase);
        return -1;
    }

    OutputTracking[phraseIndex] = true;

    float probabilitySum = 0.0f;
    size_t invalidProbabilities = 0;
    for(const Candidate& candidate : usableOutputs){
        if(candidate.output.probability <= 0.0f){
            ++invalidProbabilities;
            continue;
        }

        probabilitySum += candidate.output.probability;
    }

    if(probabilitySum > 1.0f){
        for(Candidate& candidate : usableOutputs){
            if(candidate.output.probability <= 0.0f){
                continue;
            }

            candidate.output.probability /= probabilitySum;
        }
    }else if(probabilitySum < 1.0f){
        if(invalidProbabilities > 0){
            //Normalizing p=0 candidates
            for(Candidate& candidate : usableOutputs){
                if(candidate.output.probability > 0.0f){
                    continue;
                }

                candidate.output.probability = (1.0f - probabilitySum) / invalidProbabilities;
            }
        }else{
            //Normalizing when only "rare" candidates are present
            for(Candidate& candidate : usableOutputs){
                candidate.output.probability /= probabilitySum;
            }
        }
    }

    const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    probabilitySum = 0.0f;

    for(const Candidate& candidate : usableOutputs){
        probabilitySum += candidate.output.probability;

        if(random <= probabilitySum){
            returned[candidate.index] = true;
            return candidate.index;
        }
    }

    //Fallback when rand() is outside the limits of floating-point sum precision (maybe sum isn't exactly 1.0f, instead something like 0.9999987f)
    const Candidate& last = usableOutputs.back();
    returned[last.index] = true;
    return last.index;
}
