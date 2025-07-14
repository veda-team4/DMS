#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <iostream>
#include <vector>

#include "threads.h"
#include "global.h"
#include "utils.h"

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

// ********** 제스처 감지 관련 상수 **********
constexpr uint8_t DIFF_THRESHOLD = 40;
constexpr int MORPH_ITERATIONS = 4;

constexpr double MOTION_THRESHOLD = 0.02;
constexpr int MOTION_TRIGGER_SCORE = 3;
// *******************************************

// 제스처 감지 쓰레드
void runGestureDetectionThread() {
  cv::Mat localFrame, prevFrame, gray, diff, diffBinary;

  // 손 좌우 움직이기 감지용 변수
  double prevMotionCenter = 0.0;
  int motionScore = 0;

  std::chrono::time_point<std::chrono::steady_clock> latestTime;
  {
    std::lock_guard<std::mutex> lock(frameMutex);
    leftTime = rightTime = pauseTime = latestTime = std::chrono::steady_clock::now();
  }

  while (running) {
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      if (sharedFrame.empty()) continue;
      sharedFrame.copyTo(prevFrame);
    }
    cv::cvtColor(prevFrame, prevFrame, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(prevFrame, prevFrame, cv::Size(5, 5), 0);
    break;
  }

  while (running) {
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      if (sharedFrame.empty()) {
        continue;
      }
      sharedFrame.copyTo(localFrame); // sharedFrame localFrame에 복사
    }

    // 복사해온 프레임 gray scale로 변환
    cv::cvtColor(localFrame, gray, cv::COLOR_BGR2GRAY);
    // 가우시안 필터 적용하여 노이즈 감소
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    // 현재 프레임과 평균 프레임의 차이의 절댓값 갖는 프레임 계산
    cv::absdiff(gray, prevFrame, diff);
    // 프레임 이진화
    cv::threshold(diff, diffBinary, DIFF_THRESHOLD, 255, cv::THRESH_BINARY);
    // 모폴로지 연산. 침식 -> 팽창. 작은 노이즈 제거
    cv::erode(diffBinary, diffBinary, cv::Mat(), cv::Point(-1, -1), MORPH_ITERATIONS);
    cv::dilate(diffBinary, diffBinary, cv::Mat(), cv::Point(-1, -1), MORPH_ITERATIONS);

    // X축 히스토그램 계산
    std::vector<int> histogram(diffBinary.cols, 0);
    for (int y = 0; y < diffBinary.rows; ++y) {
      for (int x = 0; x < diffBinary.cols; ++x) {
        if (diffBinary.at<uchar>(y, x) > 0) {
          ++histogram[x];
        }
      }
    }

    // left right 감지
    long long weightedSum = 0;
    int totalWeight = 0;

    // 가중 산술 평균에 필요한 값 구하기
    for (int x = 0; x < diffBinary.cols; ++x) {
      weightedSum += x * histogram[x];
      totalWeight += histogram[x];
    }

    if (totalWeight > 0) {
      // 0 ~ 1 값으로 변환
      double motionCenter = weightedSum / totalWeight;
      motionCenter = motionCenter / (histogram.size() - 1);
      // 가중 평균 변화량 구하기
      double dx = motionCenter - prevMotionCenter;
      prevMotionCenter = motionCenter;

      // 가중 평균 변화량이 임계값 이상이면 motionScore 늘이기
      if (motionCenter >= 0.8 && dx > MOTION_THRESHOLD) {
        motionScore = (motionScore >= 0) ? motionScore + 1 : 0;
      }
      // 가중 평균 변화량이 -임계값 미만이면 motionScore 늘이기
      else if (motionCenter <= 0.2 && dx < -MOTION_THRESHOLD) {
        motionScore = (motionScore <= 0) ? motionScore - 1 : 0;
      }

      // motionScore이 임계값 넘을 시 leftTime / rightTime + latestTime 갱신하기
      if (motionScore >= MOTION_TRIGGER_SCORE) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - latestTime).count() >= 1000ll) {
          {
            std::lock_guard<std::mutex> lock(timeMutex);
            leftTime = latestTime = std::chrono::steady_clock::now();
          }
        }
        motionScore = 0;
      }
      else if (motionScore <= -MOTION_TRIGGER_SCORE) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - latestTime).count() >= 1000ll) {
          {
            std::lock_guard<std::mutex> lock(timeMutex);
            rightTime = latestTime = std::chrono::steady_clock::now();
          }
        }
        motionScore = 0;
      }
    }
    else {
      motionScore = 0;
    }

    gray.copyTo(prevFrame);

    // 실시간으로 영상을 받아오는게 아니기 때문에 딜레이 필요
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
}