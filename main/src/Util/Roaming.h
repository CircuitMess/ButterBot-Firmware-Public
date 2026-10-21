#ifndef BUTTERBOT_FIRMWARE_ROAMING_H
#define BUTTERBOT_FIRMWARE_ROAMING_H

#include <atomic>

// Idle roaming, set from the controller's Settings (Ctrl::RoamingOn / RoamingOff, re-sent on every connect).
// Off: no WanderRoutine, and PersonRoutine only looks straight ahead instead of turning and driving to a face.
// Commands (voice movement, Summon, Dance) still move the robot.
namespace Roaming {
	inline std::atomic<bool> enabled{ true };
}

#endif //BUTTERBOT_FIRMWARE_ROAMING_H
