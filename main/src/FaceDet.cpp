#include "FaceDet.h"

DEFINE_LOG(FaceDet)

FaceDet::FaceDet(Camera* camera) : camera(camera){
	threadGate = xEventGroupCreate();
	// RunBit starts cleared so the thread blocks at its gate as soon as it begins.

	thread = std::make_unique<Threaded>([this]() {
		threadFunc();
	}, "FaceDet", 0, 3 * 1024, 5, 0, true);

	thread->start();

	xEventGroupWaitBits(threadGate, AtGateBit, pdFALSE, pdTRUE, portMAX_DELAY);
}

FaceDet::~FaceDet(){
	// Set RunBit so any thread parked at the gate unblocks; otherwise the Threaded destructor's
	// stop() would wait forever for a thread that is blocked on our event group.
	xEventGroupSetBits(threadGate, RunBit);
	thread->stop();

	xEventGroupClearBits(threadGate, RunBit);
	xEventGroupSetBits(threadGate, AtGateBit);
	unloadModel();
	vEventGroupDelete(threadGate);
}

void FaceDet::start(bool stopAfterDetection, bool enrollFace){
	if(!human_face_detect){
		CMF_LOG(FaceDet, LogLevel::Error, "Model not loaded!");
		return;
	}

	this->stopAfterDetection = stopAfterDetection;
	enrollNextFace = enrollFace;

	xEventGroupClearBits(threadGate, AtGateBit);
	xEventGroupSetBits(threadGate, RunBit);
}

void FaceDet::stop(){
	enrollNextFace = false;
	xEventGroupClearBits(threadGate, RunBit);
	xEventGroupWaitBits(threadGate, AtGateBit, pdFALSE, pdTRUE, portMAX_DELAY);
}

bool FaceDet::isStarted() const{
	return (xEventGroupGetBits(threadGate) & RunBit) != 0;
}

bool FaceDet::isParked() const{
	return (xEventGroupGetBits(threadGate) & (RunBit | AtGateBit)) == AtGateBit;
}

void FaceDet::loadModel(){
	if(human_face_detect){
		CMF_LOG(FaceDet, LogLevel::Error, "Model already loaded!");
		return;
	}

	if(!isParked()){
		CMF_LOG(FaceDet, LogLevel::Error, "Cannot load model while running!");
		return;
	}

	uint32_t startMillis = millis();

	human_face_detect = std::make_unique<HumanFaceDetect>();
	human_face_recognizer = std::make_unique<HumanFaceRecognizer>(db_path);

	CMF_LOG(FaceDet, Debug, "load time: %d", (int)millis() - (int)startMillis);
}

void FaceDet::unloadModel(){
	if(!human_face_detect) return;

	if(!isParked()){
		CMF_LOG(FaceDet, LogLevel::Error, "Cannot unload model while running!");
		return;
	}
	uint32_t startMillis = millis();

	human_face_detect.reset();
	human_face_recognizer.reset();
	CMF_LOG(FaceDet, Debug, "unload time: %d", (int)millis() - (int)startMillis);
}

bool FaceDet::knowsFace(){
	if(!human_face_recognizer) return false;
	return human_face_recognizer->get_num_feats() > 0;
}

bool FaceDet::forgetFace(){
	if(!isParked()){
		CMF_LOG(FaceDet, LogLevel::Error, "Cannot forget face while FaceDet is running");
		return false;
	}
	if(!knowsFace()) return false;

	human_face_recognizer->clear_all_feats();
	return true;
}

void FaceDet::learnFace(){
	if(!isParked()){
		CMF_LOG(FaceDet, LogLevel::Error, "Cannot learn face while FaceDet is running");
		return;
	}

	start(true, true);
}

void FaceDet::threadFunc(){
	if((xEventGroupGetBits(threadGate) & RunBit) == 0){
		xEventGroupSetBits(threadGate, AtGateBit);
		xEventGroupWaitBits(threadGate, RunBit, pdFALSE, pdTRUE, portMAX_DELAY);
		xEventGroupClearBits(threadGate, AtGateBit);
	}

	camera_fb_t* frameData = camera->getFrame();
	if(frameData == nullptr || frameData->buf == nullptr || frameData->len == 0){
		CMF_LOG(FaceDet, LogLevel::Error, "Couldnt get frame");
		camera->releaseFrame();
		return;
	}

	dl::image::img_t img{ frameData->buf, (uint16_t)frameData->width, (uint16_t)frameData->height, dl::image::DL_IMAGE_PIX_TYPE_RGB565 };

	detectionProcess(img);

	camera->releaseFrame();
}

void FaceDet::detectionProcess(const dl::image::img_t& img){
	if(!human_face_detect){
		CMF_LOG(FaceDet, LogLevel::Error, "Model not loaded in detection task");
		return;
	}

	auto& detect_results = human_face_detect->run(img);

	if(detect_results.empty()) return;

	const auto largestBox = std::ranges::max_element(detect_results,
	                                                 [](const dl::detect::result_t& a, const dl::detect::result_t& b) -> bool {
		                                                 return a.box_area() > b.box_area();
	                                                 });

	glm::vec2 pos = { (float)largestBox->box[0] / FrameSize.x, (float)largestBox->box[1] / FrameSize.y };
	glm::vec2 size = { (float)(largestBox->box[2] - largestBox->box[0]) / FrameSize.x, (float)(largestBox->box[3] - largestBox->box[1]) / FrameSize.y };
	CMF_LOG(FaceDet, Info, "detected face");

	if(enrollNextFace){
		CMF_LOG(FaceDet, Info, "stored face");
		ESP_ERROR_CHECK_WITHOUT_ABORT(human_face_recognizer->enroll(img, detect_results));
		enrollNextFace = false;
		OnDetect.broadcast(detect_results.size(), true, pos, size);
	} else{
		bool recognized = false;

		if(!knowsFace()){
			recognized = false;
		} else{
			const auto recognize_results = human_face_recognizer->recognize(img, detect_results);
			recognized = !recognize_results.empty();
		}

		OnDetect.broadcast(detect_results.size(), recognized, pos, size);
	}

	if(stopAfterDetection){
		// Self-pause: clear RunBit. The thread parks itself at the gate on the next iteration.
		xEventGroupClearBits(threadGate, RunBit);
	}
}
