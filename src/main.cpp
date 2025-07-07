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

// ---------------------- 전역 상수 ----------------------
#define EAR_THRESH_VAL 0.30 // 감은 눈 EAR + (뜬 눈 EAR - 감은 눈 EAR) * X
#define BLINK_RATIO_THRESH 0.6 // 눈 감은 비율 임계값
#define BLINK_WINDOW_MS 2000 // 분석 시간 윈도우 (2초)

#define INCREASE_THRESH = 2.0 // 고개 움직임 감지 최소 값
#define INCREASE_THRESH2 = 0.1 // 고개 움직임 미감지 최대 값 (이 값보다 적게 아래로 내려가면 떨어짐 아님)
#define INCREASE_TIME = 0.3 // 고개 움직임 감지 최대 시간 값
#define MAX_DOWN_COUNT = 5 // 몇번 카운트 해야 경고할 것인지
// --------------------------------------------------------

// ---------------------- 전역 변수 ----------------------
cv::VideoCapture cap;

dlib::shape_predictor sp; // 얼굴 랜드마크 추출하는 객체
dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
bool hasFace = false; // 얼굴 감지 유무
std::mutex faceMutex; // 위 두 변수 위한 뮤텍스
cv::Mat sharedFrame; // 쓰레드에게 전달하는 프레임
std::mutex frameMutex; // 위 변수 위한 뮤텍스
std::atomic<bool> running = true; // 쓰레드 동작 제어 변수

std::vector<int> landmarkIdx = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59 }; // 사용할 얼굴 랜드마크

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
  writeLog("Waiting for client ....");

  // 3. 클라이언트 연결
  client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }
  writeLog("Client connected !");

  // 4. 카메라 열기
  cap.open(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    writeLog("Camera open failed");
    return -1;
  }

  // 5. 얼굴 탐지 쓰레드 생성
  std::thread faceThread(runFaceDetectionThread);

  // 6. 루프 돌며 클라이언트로부터 명령 받아서 실행
  double thresholdEAR;
  while (true) {
    uint8_t type;
    uint32_t dataLen;
    char buf[64] = { 0, };
    readNBytes(client_fd, &type, 1);
    if (type == COMMAND) {
      readNBytes(client_fd, &dataLen, 4);
      readNBytes(client_fd, buf, dataLen);
      writeLog(std::string("message from client: ") + std::string(buf));
      if (strcmp(buf, "camset") == 0) {
        if (camsetpage() == -1) {
          writeLog("camsetpage error");
          break;
        }
      }
      else if (strcmp(buf, "calibrate") == 0) {
        if (calibratepage(thresholdEAR) == -1) {
          writeLog("calibrate error");
          break;
        }
      }
      else if (strcmp(buf, "monitor") == 0) {
        if (monitorpage(thresholdEAR) == -1) {
          writeLog("monitor error");
          break;
        }
      }
      else if (strcmp(buf, "stop") == 0) {
        break;
      }
    }
    else {
      writeLog("Undefined situation");
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