#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <iostream>
#include "threads.h"

int camsetpage() {
  // 쓰레드와의 공유 변수
  dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
  bool hasFace = false; // 얼굴 감지 여부
  std::mutex faceMutex; // 위 두 변수 동기화 뮤텍스
  cv::Mat sharedFrame; // 쓰레드에게 전달할 프레임
  std::mutex frameMutex; // 위 변수 동기화 뮤텍스
  std::atomic<bool> running = true; // 프로그램 동작 여부 변수 (쓰레드 제어용)

  // 얼굴 탐지 스레드 시작
  dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
  std::thread faceThread(runFaceDetectionThread,
    std::ref(running), std::ref(detector), std::ref(sharedFrame),
    std::ref(frameMutex), std::ref(biggestFaceRect), std::ref(hasFace),
    std::ref(faceMutex));

  int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return -1;
  }

  std::string socketPath = std::string(getenv("HOME")) + "/.face_socket";

  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
  unlink(socketPath.c_str());

  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return -1;
  }

  listen(server_fd, 1);
  std::cout << "Waiting for connection..." << std::endl;
  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }

  cv::VideoCapture cap(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Camera open failed!" << std::endl;
    return -1;
  }

  while (true) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) break;

    // 얼굴 영역 계산 쓰레드와 최신 프레임 공유
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      frame.copyTo(sharedFrame);
    }

    // 얼굴 박스 가져오기
    dlib::rectangle faceRect;
    bool localHasFace;
    {
      std::lock_guard<std::mutex> lock(faceMutex);
      localHasFace = hasFace;
      faceRect = biggestFaceRect;
    }

    if (localHasFace) {
      // 얼굴 박스 표시
      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);
    }

    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    uint32_t size = buf.size();

    if (write(client_fd, &size, sizeof(size)) != sizeof(size)) break;
    if (write(client_fd, buf.data(), size) != size) break;
  }

  close(client_fd);
  close(server_fd);
  unlink(socketPath.c_str());
  running = false;
  faceThread.join();

  return 0;
}