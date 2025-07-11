#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include "threads.h"
#include "global.h"
#include "utils.h"
#include "protocols.h"

int monitorpage(double thresholdEAR) {
  // 눈 감음 정보 저장 변수
  std::deque<std::pair<std::chrono::steady_clock::time_point, bool>> blinkHistory;
  unsigned long long closedCount = 0;
  double eyeClosedRatio = 0.0; // BLINK_WINDOW_MS 중에 눈 감은 비율

  // 고개 떨어짐 정보 저장 변수
  int downCount = 0; // 고개 떨어짐 횟수
  double earAvg = 0.0; // 양쪽 눈의 EAR 평균

  while (true) {
    // 클라이언트 측으로부터 "stop" 수신 시 종료
    uint8_t protocol;
    protocol = readEncryptedCommandNonBlock(client_fd);
    if (protocol != Protocol::NOTHING) {
      if (protocol == Protocol::STOP) {
        writeLog("message from client: STOP");
        return 0;
      }
      else {
        return -1;
      }
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) break;

    // 얼굴 탐지 쓰레드를 위해 최신 프레임 공유
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      frame.copyTo(sharedFrame);
    }

    // 얼굴 사각형과 감지 유무 받아옴
    dlib::rectangle faceRect;
    bool localHasFace;
    {
      std::lock_guard<std::mutex> lock(faceMutex);
      localHasFace = hasFace;
      faceRect = biggestFaceRect;
    }

    // 얼굴 감지 성공시
    if (localHasFace) {
      // 68개의 얼굴 랜드마크 추출
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      // 얼굴 사각형 그리기
      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);

      // 랜드마크 이용해서 EAR 계산
      double earL = computeEAR(landmarks, 36);
      double earR = computeEAR(landmarks, 42);
      earAvg = (computeEAR(landmarks, 36) + computeEAR(landmarks, 42)) / 2.0;

      // 고개 떨어짐 계산. 27 ~ 30 코 랜드마크 평균 y값을 계산하여 이전 프레임의 평균 y값과 비교
      static double prevNoseY = 1e50; // 이전 고개 좌표
      double currentNose = 0.0;
      for (int i = 27; i <= 30; ++i) {
        currentNose += landmarks.part(30).y();
      }
      currentNose /= 4;

      double diff = currentNose - prevNoseY;
      static auto prevTime = std::chrono::steady_clock::now();
      auto currTime = std::chrono::steady_clock::now();
      double deltaTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime).count() / 1000.0;

      // 이전 좌표와 비교해서 INCREASE_THRESH 보다 크게 증가하면 downCount 증가
      if (diff >= INCREASE_THRESH && deltaTimeSec < INCREASE_TIME) {
        ++downCount;
      }
      // 이전 좌표와 비교해서 INCREASE_THRESH 보다 작게 증가하면 downCount 0 으로 초기화
      else if (diff < INCREASE_THRESH2) {
        downCount = 0;
      }
      prevNoseY = currentNose;
      prevTime = currTime;

      // 눈 감음 여부 계산. 감았을 시 closedCount 증가 및 경고 문구 출력
      bool isClosed = (earAvg < thresholdEAR);
      if (isClosed) {
        ++closedCount;
        cv::putText(frame, "CLOSED",
          cv::Point(faceRect.left(), faceRect.top() - 10),
          cv::FONT_HERSHEY_SIMPLEX, 1.0,
          cv::Scalar(0, 0, 255), 2);
      }

      // { 현재 시간, 눈 감음 여부} 기록
      auto now = std::chrono::steady_clock::now();
      blinkHistory.emplace_back(now, isClosed);
    }
    else {
      blinkHistory.emplace_back(std::chrono::steady_clock::now(), false);
    }

    // 2초간의 윈도우 초과한 항목 제거
    while (!blinkHistory.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(blinkHistory.back().first - blinkHistory.front().first).count() > BLINK_WINDOW_MS) {
      if (blinkHistory.front().second) --closedCount;
      blinkHistory.pop_front();
    }

    // 2초간의 { 현재 시간, 눈 감음 여부 } Window 에서 눈 감김 비율 계산
    if (!blinkHistory.empty()) {
      eyeClosedRatio = static_cast<double>(closedCount) / blinkHistory.size();
    }

    // 눈 감김 비율 전송
    protocol = Protocol::EYECLOSEDRATIO;
    if (writeEncryptedData(client_fd, protocol, eyeClosedRatio) == -1) {
      return -1;
    }

    // 머리 떨어짐 감지 및 출력
    if (downCount >= MAX_DOWN_COUNT) {
      if (writeEncryptedCommand(client_fd, Protocol::HEADDROPPED) == -1) {
        return -1;
      }
    }

    // 클라이언트에 프레임 전송하기
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    if (writeEncryptedFrame(client_fd, buf) == -1) {
      return -1;
    }
  }

  return 0;
}