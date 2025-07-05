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

int calibratepage() {
  // Open your eyes 단계
  while (true) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) continue;

    // send frame to client
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    uint32_t size = buf.size();

    if (write(client_fd, &size, sizeof(size)) != sizeof(size)) break;
    if (write(client_fd, buf.data(), size) != size) break;
  }

  /*
  double openEarSum = 0.0;
  int openEarCount = 0;
  auto openStart = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - openStart < std::chrono::seconds(2)) {
    cv::Mat tempFrame;
    cap >> tempFrame;
    if (tempFrame.empty()) continue;

    dlib::cv_image<dlib::bgr_pixel> dlibFrame(tempFrame);
    std::vector<dlib::rectangle> faces = detector(dlibFrame);
    if (!faces.empty()) {
      dlib::full_object_detection landmarks = sp(dlibFrame, faces[0]);

      cv::rectangle(tempFrame,
        cv::Point(faces[0].left(), faces[0].top()),
        cv::Point(faces[0].right(), faces[0].bottom()),
        cv::Scalar(0, 255, 0), 2);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(tempFrame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }

      double ear = (computeEAR(landmarks, 36) + computeEAR(landmarks, 42)) / 2.0;
      openEarSum += ear;
      openEarCount++;
    }
    double secondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - openStart).count() / 1000.0;
    std::string timerText = "Time: " + std::to_string(secondsElapsed).substr(0, 4) + "s";
    cv::putText(tempFrame, timerText, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    cv::imshow("Frame", tempFrame);
    cv::waitKey(1);
  }
  double openEAR = (openEarCount > 0) ? openEarSum / openEarCount : 0.3;

  // Close your eyes 단계
  while (true) {
    cv::Mat tempFrame;
    cap >> tempFrame;
    if (tempFrame.empty()) continue;
    cv::putText(tempFrame, "Close your eyes.", cv::Point(30, 50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2); cv::imshow("Frame", tempFrame);
    cv::putText(tempFrame, "Press ENTER when you are ready", cv::Point(30, 100), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2); cv::imshow("Frame", tempFrame);
    dlib::cv_image<dlib::bgr_pixel> dlibFrame(tempFrame);
    std::vector<dlib::rectangle> faces = detector(dlibFrame);
    if (!faces.empty()) {
      dlib::full_object_detection landmarks = sp(dlibFrame, faces[0]);

      cv::rectangle(tempFrame,
        cv::Point(faces[0].left(), faces[0].top()),
        cv::Point(faces[0].right(), faces[0].bottom()),
        cv::Scalar(0, 255, 0), 2);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(tempFrame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }
    }
    cv::imshow("Frame", tempFrame);
    if (cv::waitKey(30) == 13) break;
  }

  double closeEarSum = 0.0;
  int closeEarCount = 0;
  auto closeStart = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - closeStart < std::chrono::seconds(2)) {
    cv::Mat tempFrame;
    cap >> tempFrame;
    if (tempFrame.empty()) continue;

    dlib::cv_image<dlib::bgr_pixel> dlibFrame(tempFrame);
    std::vector<dlib::rectangle> faces = detector(dlibFrame);
    if (!faces.empty()) {
      dlib::full_object_detection landmarks = sp(dlibFrame, faces[0]);

      cv::rectangle(tempFrame,
        cv::Point(faces[0].left(), faces[0].top()),
        cv::Point(faces[0].right(), faces[0].bottom()),
        cv::Scalar(0, 255, 0), 2);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(tempFrame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }

      double ear = (computeEAR(landmarks, 36) + computeEAR(landmarks, 42)) / 2.0;
      closeEarSum += ear;
      closeEarCount++;
    }
    double secondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - closeStart).count() / 1000.0;
    std::string timerText = "Time: " + std::to_string(secondsElapsed).substr(0, 4) + "s";
    cv::putText(tempFrame, timerText, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    cv::imshow("Frame", tempFrame);
    cv::waitKey(1);
  }
  double closeEAR = (closeEarCount > 0) ? closeEarSum / closeEarCount : 0.2;

  double EAR_THRESH = closeEAR + (openEAR - closeEAR) * 0.2;
  std::string str1 = "Open EAR: " + std::to_string(openEAR) + ", Close EAR: " + std::to_string(closeEAR);
  std::string str2 = "Threshold: " + std::to_string(EAR_THRESH);

  // 최종 시작 전 대기
  while (true) {
    cv::Mat tempFrame;
    cap >> tempFrame;
    if (tempFrame.empty()) continue;
    cv::putText(tempFrame, "Calibration done. Press ENTER to start app", cv::Point(30, 50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    cv::putText(tempFrame, str1, cv::Point(30, 100), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    cv::putText(tempFrame, str2, cv::Point(30, 150), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    cv::imshow("Frame", tempFrame);
    if (cv::waitKey(30) == 13) break;
  }
  */
  return 0;
}