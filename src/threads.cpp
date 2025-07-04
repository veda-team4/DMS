#include "threads.h"

extern dlib::shape_predictor sp;
extern dlib::rectangle biggestFaceRect; // Biggest face rectangle
extern bool hasFace; // Whether face recongition
extern std::mutex faceMutex; // Mutex for the above two variables
extern cv::Mat sharedFrame; // Frame for thread
extern std::mutex frameMutex; // Mutex for the above variable
extern std::atomic<bool> running; // Control whether thread runs

void runFaceDetectionThread() {
  dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
  dlib::deserialize("../model/shape_predictor_68_face_landmarks.dat") >> sp;
  while (running) {
    cv::Mat localFrame;
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      if (sharedFrame.empty()) continue;
      sharedFrame.copyTo(localFrame);
    }

    dlib::cv_image<dlib::bgr_pixel> dlibFrame(localFrame);
    std::vector<dlib::rectangle> faces = detector(dlibFrame);

    {
      std::lock_guard<std::mutex> lock(faceMutex);
      if (faces.empty()) {
        hasFace = false;
      }
      else if (faces.size() == 1) {
        biggestFaceRect = faces[0];
        hasFace = true;
      }
      else {
        biggestFaceRect = *std::max_element(faces.begin(), faces.end(),
          [](const dlib::rectangle& a, const dlib::rectangle& b) {
            return a.area() < b.area();
          });
        hasFace = true;
      }
    }
  }
}