#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "threads.h"
#include "functions.h"

#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

// Shared variable declaration
cv::VideoCapture cap;
dlib::shape_predictor sp;

dlib::rectangle biggestFaceRect; // Biggest face rectangle
bool hasFace = false; // Whether face recongition
std::mutex faceMutex; // Mutex for the above two variables
cv::Mat sharedFrame; // Frame for thread
std::mutex frameMutex; // Mutex for the above variable
std::atomic<bool> running = true; // Control whether thread runs

int server_fd, client_fd;

int main(void) {
  // 0. Create UNIX Domain Socket and accept client.
  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket()");
    return -1;
  }

  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
  unlink(SOCKET_PATH);

  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind()");
    return -1;
  }

  listen(server_fd, 1);
  std::cout << "Waiting for client ...." << std::endl;

  client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }
  std::cout << "Client connected !" << std::endl;

  // 1. Set camera
  cap.open(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Camera open failed\n";
    return -1;
  }

  // 2. Run face detection thread
  std::thread faceThread(runFaceDetectionThread);

  while (true) {
    uint8_t lenByte;
    ssize_t lenRead = read(client_fd, &lenByte, 1);
    if (lenRead == 1) {
      if (lenByte == 0 || lenByte >= 64) {
        std::cout << "Invalid command length" << std::endl;
        return -1;
      }
      char cmdBuf[64] = { 0 };
      ssize_t totalRead = 0;
      while (totalRead < lenByte) {
        ssize_t n = read(client_fd, cmdBuf + totalRead, lenByte - totalRead);
        if (n > 0) {
          totalRead += n;
        }
        else if (n == 0) {
          std::cout << "Client disconnected.\n";
          break;
        }
        else {
          perror("read");
          break;
        }
      }
      if (totalRead != lenByte) continue;

      std::string command(cmdBuf, lenByte);

      std::cout << "Received command: " << command << std::endl;

      if (command == "camset") {
        camsetpage();
      }
      else if (command == "calibrate") {
        calibratepage();
      }
      else if (command == "stop") {
        break;
      }
    }
    else if (lenRead == 0) {
      std::cout << "Client disconnected" << std::endl;
      break;
    }
    else {
      perror("read()");
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