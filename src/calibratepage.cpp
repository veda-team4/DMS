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
#include "protocols.h"
#include "utils.h"

int waitingPage(const char*);
int calibrateEyes(double&);

int calibratepage(double& averageEAR) {
  // 뜬 눈 EAR, 감은 눈 EAR
  double openedEAR, closedEAR;

  // "opened" 수신 할 때까지 영상 전송
  if (waitingPage("opened") == -1) {
    return -1;
  }

  // 뜬 눈 EAR 계산
  if (calibrateEyes(openedEAR) == -1) {
    return -1;
  }

  // "closed" 수신 할 때까지 영상 전송
  if (waitingPage("closed") == -1) {
    return -1;
  }

  // 감은 눈 EAR 계산
  if (calibrateEyes(closedEAR) == -1) {
    return -1;
  }

  averageEAR = closedEAR + (openedEAR - closedEAR) * 0.2;
  return 0;
}

int waitingPage(const char* msg) {
  // 클라이언트 측으로부터 msg 수신할 때까지 프레임 전송
  while (true) {
    uint8_t type;
    if (recv(client_fd, &type, 1, MSG_DONTWAIT) == 1) {
      if (type == COMMAND) {
        uint32_t dataLen;
        char buf[64] = { 0, };
        readNBytes(client_fd, &dataLen, 4);
        readNBytes(client_fd, buf, dataLen);
        std::cout << "[Server] message from client: " << buf << std::endl;
        // 클라이언트 측으로부터 msg 수신 시 while 문 빠져나감
        if (strcmp(buf, msg) == 0) {
          break;
        }
        else {
          return -1;
        }
      }
      else {
        return -1;
      }
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) continue;

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

    if (localHasFace) {
      // 얼굴 사각형 그리기
      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);

      // 랜드마크 그리기
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(frame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }
    }

    // 클라이언트에 프레임 전송하기
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    uint32_t size = buf.size();
    uint8_t protocol = VIDEO;

    if (writeNBytes(client_fd, &protocol, 1) == -1) return -1;
    if (writeNBytes(client_fd, &size, 4) == -1) return -1;
    if (writeNBytes(client_fd, buf.data(), size) == -1) return -1;
  }

  return 0;
}

int calibrateEyes(double& opened) {
  // 클라이언트 측으로부터 finish 수신할 때까지 프레임 전송
  while (true) {
    uint8_t type;
    if (recv(client_fd, &type, 1, MSG_DONTWAIT) == 1) {
      if (type == COMMAND) {
        uint32_t dataLen;
        char buf[64] = { 0, };
        readNBytes(client_fd, &dataLen, 4);
        readNBytes(client_fd, buf, dataLen);
        std::cout << "[Server] message from client: " << buf << std::endl;
        // 클라이언트 측으로부터 finish 수신 시 while 문 빠져나감
        if (strcmp(buf, "finish") == 0) {
          break;
        }
        else {
          return -1;
        }
      }
      else {
        return -1;
      }
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) continue;

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

    if (localHasFace) {
      // 얼굴 사각형 그리기
      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);

      // 랜드마크 그리기
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(frame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }
    }

    // 클라이언트에 프레임 전송하기
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    uint32_t size = buf.size();
    uint8_t protocol = VIDEO;

    if (writeNBytes(client_fd, &protocol, 1) == -1) return -1;
    if (writeNBytes(client_fd, &size, 4) == -1) return -1;
    if (writeNBytes(client_fd, buf.data(), size) == -1) return -1;
  }

  return 0;
}
