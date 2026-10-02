#ifndef BUTTERBOT_FIRMWARE_FACEDET_H
#define BUTTERBOT_FIRMWARE_FACEDET_H

#include "Entity/AsyncEntity.h"
#include "Devices/Camera.h"
#include "Event/EventBroadcaster.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <glm.hpp>
#include <human_face_detect.hpp>
#include <human_face_recognition.hpp>
#include <memory>

#include "FileSystem/SPIFFS.h"

class FaceDet : public Object {
	GENERATED_BODY(FaceDet, Object, CONSTRUCTOR_PACK(Camera*))
public:
	FaceDet(Camera* camera);

	~FaceDet() override;

	void start(bool stopAfterDetection, bool enrollFace = false);
	void stop();
	bool isStarted() const;

	void loadModel();
	void unloadModel();


	/**
	 * Check if a recognized face is already enrolled and stored in memory.
	 * @return
	 */
	bool knowsFace();

	/**
	 * Forget the enrolled recognized face from memory.
	 * @return true - face succesfully forgotten, false - no face available to forget
	 */
	bool forgetFace();


	void learnFace();


	/**
	 *  int faceCount - number of faces detected (>= 0)
	 *  bool known - a known face is detected
	 *  glm::vec2 pos - normalized position of top-left corner of face bounding rectangle
	 *  glm::vec2 size - normalized size of face bounding rectangle
	 *
	 * Note: if faceCount > 1, then pos and size will be of the largest bounding rect face,
	 * or the known face (if known == True)
	 */
	DECLARE_EVENT(Event, FaceDet, int, bool, glm::vec2, glm::vec2);

	Event OnDetect{ this };

private:
	std::unique_ptr<Threaded> thread;
	StrongObjectPtr<Camera> camera;

	/**
	 * Coordinates whether the detection thread should be doing work. When RunBit is clear,
	 * the thread blocks at its gate at the start of each loop iteration; AtGateBit lets stop()
	 * wait until the thread has actually parked itself before returning.
	 */
	EventGroupHandle_t threadGate = nullptr;
	static constexpr EventBits_t RunBit = BIT0;
	static constexpr EventBits_t AtGateBit = BIT1;

	/**
	 * True only when the detection thread is actually parked at its gate (AtGateBit set and RunBit
	 * clear). This, not isStarted(), is the condition under which the models may be touched from
	 * another thread: isStarted() only reflects the request, the thread may still be mid-inference.
	 */
	bool isParked() const;

	void threadFunc();
	void detectionProcess(const dl::image::img_t& img);

	char db_path[64] = "/spiffs/face.db";
	std::unique_ptr<HumanFaceDetect> human_face_detect;
	std::unique_ptr<HumanFaceRecognizer> human_face_recognizer;

	std::atomic_bool stopAfterDetection = false;
	std::atomic_bool enrollNextFace = false;

	static constexpr glm::vec2 FrameSize = { 128, 128 };
};


#endif //BUTTERBOT_FIRMWARE_FACEDET_H
