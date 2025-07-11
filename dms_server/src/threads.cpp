#include "threads.h"
#include "global.h"

// 얼굴 검출 쓰레드
void runFaceDetectionThread() {
  // 얼굴 및 랜드마크 검출 객체 할당
  dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
  dlib::deserialize("../../dms_server/model/shape_predictor_68_face_landmarks.dat") >> sp;

  // 반복문 돌며 얼굴 검출하기
  while (running) {
    cv::Mat localFrame; // 쓰레드에서 사용할 프레임 변수
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      if (sharedFrame.empty()) continue;
      sharedFrame.copyTo(localFrame); // sharedFrame localFrame에 복사
    }

    // localFrame dlibFrame 으로 변환 후 얼굴 검출
    dlib::cv_image<dlib::bgr_pixel> dlibFrame(localFrame);
    std::vector<dlib::rectangle> faces = detector(dlibFrame);

    // 검출된 가장 큰 얼굴 사각형 갱신
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