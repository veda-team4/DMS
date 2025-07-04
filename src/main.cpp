#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "threads.h"
#include "functions.h"

#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

cv::VideoCapture cap;
dlib::frontal_face_detector detector;
dlib::shape_predictor sp;

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
  // ----------------------------------------------

  // 1. Set camera and dlib library
  cap.open(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Camera open failed\n";
    return -1;
  }
  detector = dlib::get_frontal_face_detector();
  dlib::deserialize("../model/shape_predictor_68_face_landmarks.dat") >> sp;
  // ----------------------------------------------

  camsetpage();

  // 1. Create camera input thread.
  // Receive frames from the camera and perform face recognition and landmark calculations.

  // ----------------------------------------------

  return 0;
}