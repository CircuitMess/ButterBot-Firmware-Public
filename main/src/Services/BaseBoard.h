#ifndef BUTTERBOT_FIRMWARE_BASEBOARD_H
#define BUTTERBOT_FIRMWARE_BASEBOARD_H

#include <atomic>
#include "Devices/Timer.h"
#include "Entity/AsyncEntity.h"
#include "Event/EventBroadcaster.h"
#include "Periphery/I2CDevice.h"

enum class ProxSensor : uint8_t {Front, Bottom};
enum class ProxState : uint8_t {Unknown, Covered, Uncovered};
enum class ChargingState : uint8_t {Unplugged, Charging, Full};

class BaseBoard : public AsyncEntity {
	GENERATED_BODY(BaseBoard, AsyncEntity, CONSTRUCTOR_PACK(std::unique_ptr<I2CDevice>))

public:
	BaseBoard(std::unique_ptr<I2CDevice> device, bool internalStack = true);

	void setMotors(int8_t left, int8_t right);

	void setMotorLeft(int8_t value);

	void setMotorRight(int8_t value);

	/**
	 * Sets both motors to zero.
	 */
	void stopMotors();

	/**
	 * Toggles active proximity sensor scanning.
	 * If a proximity sensor's threshold is exceeded while scanning is enabled,
	 * an "onProximityChange" event will be broadcast.
	 *
	 * @param enabled true - scanning enabled, false - scanning disabled
	 */
	void setProximityScanning(bool enabled);

	void rebaselineProximity();

	// Superseded by relative threshold logic on Base — no-op, kept for API compatibility
	void setProximityFrontThreshold(uint16_t threshold);

	void setProximityBotThreshold(uint16_t threshold);

	/**
	 * Requests a single proximity scan.
	 * Results will be returned in the following "onProximityReading" event broadcast.
	 */
	void requestProximityState();

	/**
	 * Ignore a proximity sensor. A disabled front sensor always reads "no obstacle",
	 * a disabled bottom sensor always reads "on the ground". Applies to every routine and to lift detection.
	 */
	void setProximityEnabled(bool front, bool bottom);

	ChargingState getChargingState();

	/**
	 * Event in case of a Charge state change.
	 */
	DECLARE_EVENT(ChargeStateEvent, BaseBoard, ChargingState);

	ChargeStateEvent onChargeChanged = ChargeStateEvent(this);

	/**
	 * Event which will be called following a "requestProximityState" call, with the results.
	 * ProxState front - Covered - obstacle encountered, Uncovered - nothing detected, None - state not yet determined (baseline)
	 * ProxState bot   - Covered - obstacle encountered, Uncovered - nothing detected, None - state not yet determined (baseline)
	 */
	DECLARE_EVENT(ProximityReadingEvent, BaseBoard, ProxState, ProxState);

	ProximityReadingEvent onProximityReading = ProximityReadingEvent(this);

	/**
	 * Event of a proximity sensor's threshold being reached.
	 * Will only be called if "setProximityScanning" was set to true.
	 *
	 * ProximitySensor sensor - identifier of which sensor's state was changed
	 * bool thresholdChange - true - value changed over the threshold, false - value changed under the threshold
	 */
	DECLARE_EVENT(ProximityChangeEvent, BaseBoard, ProxSensor, bool);

	ProximityChangeEvent onProximityChange = ProximityChangeEvent(this);

	struct ProximityReadingData {
		ProxState front;
		ProxState bot;
	};

	struct ProximityChangeData {
		ProxSensor sensor;
		bool inThreshold;
	};

protected:
	void tick(float deltaTime) noexcept override;

private:
	/**
	 * Resets BaseBoard's microcontroller and local register state.
	 *
	 * Note: declared as private to avoid synchronization between reset() calls and possible ongoing registerState access in BaseBoard's own thread.
	 */
	void reset();

	void updateMotorTimer();

	uint8_t getEventCount();

	std::unique_ptr<I2CDevice> dev;

	/**
	 * Motor control follows a dead-man-switch principle.
	 * If motor control is active (at least one motor's speed is non-zero),
	 * the master is expected to continuously send this data at a select frequency.
	 * If the slave fails to receive a motor control command in the specified timeframe, it stops both motors until further commands are received.
	 */
	StrongObjectPtr<Timer> motorTimer;

	static constexpr uint32_t MotorControlPeriod = 250; //[ms], must be below the defined dead-man-switch timeframe of 500ms
	static constexpr uint32_t ProxStopSettleMs = 3;
	static constexpr uint32_t ProxRebaselineDelayMs = 100;
	/**
	 * Main blocking semaphore of BaseBoard's task (thread).
	 * Will be unblocked either through the IRQ routine from BaseBoard's interrupt pin,
	 * or the periodic MotorTimer.
	 */
	static SemaphoreHandle_t sem;
	static void timerFunc(void*);

	StrongObjectPtr<Timer> proxTimer;
	static std::atomic<bool> proxRebaselineDue;
	static void proxTimerFunc(void*);

	static void IRAM_ATTR motorBoardISR(void* arg);


	static constexpr uint8_t I2CAddress = 0x69;
	/**
	 * Setup time needed between a reset and the following command [ms].
	 */
	static constexpr uint32_t ResetDelay = 200;

	struct RegisterState {
		int8_t motorLeft = 0;
		int8_t motorRight = 0;
		bool proximityScanning = false;
		// uint16_t proximityFrontThreshold = 0;   // removed — absolute thresholds superseded
		// uint16_t proximityBottomThreshold = 0;  // removed — absolute thresholds superseded
	} registerState;

	// proximity sensor filter, see setProximityEnabled()
	std::atomic<bool> proxFrontEnabled{ true };
	std::atomic<bool> proxBottomEnabled{ true };

	/**
	 * Needed for synchronization of registerState access from public methods and internal thread.
	 * Used only for locking access to motor states.
	 */
	std::mutex regStateMutex;

	/**
	 * Note: avoided using enum 'class' here for the benefit of an implicit type conversion in a common use case:
	 * i2c->write(register, data) instead of i2c->write((uint8_t)register, data)
	 */
	enum Register : uint8_t {
		Identify = 0x0,
		Reset = 0x1,
		MotorL = 0x02,
		MotorR = 0x03,
		ProximityScanning = 0x04,
		// ProximityFrontThresh = 0x05,   // removed — Base uses relative band logic
		// ProximityBottomThresh = 0x07,  // removed — Base uses relative band logic
		ProximityStateRequest = 0x09,
		ChargeState = 0x0A,
		EventCount = 0x0B,
		EventsGet = 0x0C
	};

	enum class EventType : uint8_t {
		ProximityReading,
		ProximityThresholdReached,
		ChargeStateChanged
	};

	struct EventData {
		EventType type;
		union {
			ProximityReadingData proxReading;
			ProximityChangeData proxThresholdReached;
			ChargingState chargeStateChanged;
		} data;
	};

	void processEventData(const EventData& eventData);
};


#endif //BUTTERBOT_FIRMWARE_BASEBOARD_H
