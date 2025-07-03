#include "threads.h"

void runFaceDetectionThread(std::atomic<bool>& running, dlib::frontal_face_detector& detector, cv::Mat& sharedFrame, std::mutex& frameMutex, dlib::rectangle& biggestFaceRect, bool& hasFace, std::mutex& faceMutex) {
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