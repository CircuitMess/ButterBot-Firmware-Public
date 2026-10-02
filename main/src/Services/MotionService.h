#ifndef BUTTERBOT_FIRMWARE_MOTIONSERVICE_H
#define BUTTERBOT_FIRMWARE_MOTIONSERVICE_H

#include <Object/Object.h>
#include <Devices/SC7A20.h>
#include <Entity/AsyncEntity.h>
#include "BaseBoard.h"
#include <optional>

enum class MotionType : uint8_t {
	Fall, UpsideDown, Pickup, Shake
};

class MotionService : public Object {
	GENERATED_BODY(MotionService, Object, void)
public:
	MotionService();
	~MotionService() override;

	virtual void postInitProperties() noexcept override;

	DECLARE_EVENT(MotionEvent, MotionService, MotionType);
	MotionEvent onMotion = MotionEvent(this);

	void enable();
	void disable();

	bool isUprightOnGround();

private:
	std::vector<SC7A20::Sample> fifo;
	static constexpr size_t MaxFIFODepth = 64;

	void proximityChanged(ProxSensor sensor, bool thresholdChange);
	void proximityReading(ProxState front, ProxState bottom);
	std::atomic_bool proximityLifted = false;

	std::atomic_bool processingEnabled = false;

	void loop() noexcept;

	std::unique_ptr<Threaded> thread;

	SemaphoreHandle_t sem;

	static constexpr uint32_t FIFOFillTime = 320; // [ms]

	//-----------------------------------------------------
	enum class Pose: uint8_t {
		Straight, //X = 1
		Left, //Y=1
		Right, //Y=-1
		Forward, //Z=-1
		Backward, //Z=1
		UpsideDown, //X = -1
		Moving //set by processShake, invalidates current pose
	};

	//For determining pose
	static constexpr float AxisMin = 0.65f;
	static constexpr float AxisMax = -0.65f;

	//After two seconds of the same candidate (non-shake event), an event will be broadcast.
	static constexpr uint32_t CandidateHoldTime = 2000;
	static constexpr float GrossMotionRange = 0.3f;
	static constexpr float MeanTolerance = 0.02f;
	static constexpr uint8_t StillBatchCount = 6;

	Pose detectPose(SC7A20::Sample sample);
	static float maxRange(const std::vector<SC7A20::Sample>& samples);
	static SC7A20::Sample meanSample(const std::vector<SC7A20::Sample>& samples);
	static float maxAxisDiff(const SC7A20::Sample& a, const SC7A20::Sample& b);

	Pose lastPose = Pose::Moving;

	//Used to prevent double-firing events, which aren't shake
	std::optional<MotionType> lastEvent;
	MotionType candidate = MotionType::Shake;
	uint32_t candidateStart = 0;

	bool pickupDone = false;
	//-----------------------------------------------------

	struct ShakeDetector {
		static constexpr float VarianceThreshold = 0.4f;
		static constexpr size_t WindowSize = 64;

		SC7A20::Sample window[WindowSize] = {};
		size_t writeIdx = 0;
		int count = 0;

		float sumX = 0, sumX2 = 0;
		float sumY = 0, sumY2 = 0;
		float sumZ = 0, sumZ2 = 0;

		bool isShaking = false;

		void onAdd(const SC7A20::Sample& s){
			if(count == (int)WindowSize){
				const SC7A20::Sample& e = window[writeIdx];
				sumX -= e.accelX;
				sumX2 -= e.accelX * e.accelX;
				sumY -= e.accelY;
				sumY2 -= e.accelY * e.accelY;
				sumZ -= e.accelZ;
				sumZ2 -= e.accelZ * e.accelZ;
			} else{
				count++;
			}
			window[writeIdx] = s;
			writeIdx = (writeIdx + 1) % WindowSize;
			sumX += s.accelX;
			sumX2 += s.accelX * s.accelX;
			sumY += s.accelY;
			sumY2 += s.accelY * s.accelY;
			sumZ += s.accelZ;
			sumZ2 += s.accelZ * s.accelZ;
		}

		bool detected();

		void reset(){
			sumX = sumX2 = sumY = sumY2 = sumZ = sumZ2 = 0.0f;
			count = 0;
			writeIdx = 0;
			isShaking = false;
		}
	} shake;
};


#endif //BUTTERBOT_FIRMWARE_MOTIONSERVICE_H
