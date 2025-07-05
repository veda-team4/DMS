#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "threads.h"
#include "functions.h"
#include "protocols.h"
#include "utils.h"

#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

// ---------------------- 전역 변수 ----------------------
cv::VideoCapture cap;

dlib::shape_predictor sp; // 얼굴 랜드마크 추출하는 객체
dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
bool hasFace = false; // 얼굴 감지 유무
std::mutex faceMutex; // 위 두 변수 위한 뮤텍스
cv::Mat sharedFrame; // 쓰레드에게 전달하는 프레임
std::mutex frameMutex; // 위 변수 위한 뮤텍스
std::atomic<bool> running = true; // 쓰레드 동작 제어 변수

int server_fd, client_fd; // 서버, 클라이언트 소켓 파일 디스크립터
// --------------------------------------------------------

int main(void) {
  // 0. UNIX Domain Socket 생성
  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket()");
    return -1;
  }
  // 1. 소켓 - 주소 연결
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
  unlink(SOCKET_PATH);

  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind()");
    return -1;
  }

  // 2. 클라이언트 연결 대기
  listen(server_fd, 1);
  std::cout << "[Server] Waiting for client ...." << std::endl;

  // 3. 클라이언트 연결
  client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }
  std::cout << "[Server] Client connected !" << std::endl;

  // 4. 카메라 열기
  cap.open(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Camera open failed\n";
    return -1;
  }

  // 5. 얼굴 탐지 쓰레드 생성
  std::thread faceThread(runFaceDetectionThread);

  // 6. 루프 돌며 클라이언트로부터 명령 받아서 실행
  while (true) {
    uint8_t type;
    uint32_t dataLen;
    char buf[64] = { 0, };
    readNBytes(client_fd, &type, 1);
    if (type == COMMAND) {
      readNBytes(client_fd, &dataLen, 4);
      readNBytes(client_fd, buf, dataLen);
      std::cout << "[Server] message from client: " << buf << std::endl;
      if (strcmp(buf, "camset") == 0) {
        if (camsetpage() == -1) {
          puts("camsetpage error");
          break;
        }
      }
      else if (strcmp(buf, "calibrate") == 0) {
        if (calibratepage() == -1) {
          puts("calibrate error");
          break;
        }
      }
      else if (strcmp(buf, "stop") == 0) {
        break;
      }
    }
    else {
      puts("Undefined situation");
      break;
    }
  }

  running = false;
  faceThread.join();
  close(client_fd);
  close(server_fd);
  unlink(SOCKET_PATH);
  return 0;
}