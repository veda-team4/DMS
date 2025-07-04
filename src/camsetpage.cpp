#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include "threads.h"

// Shared variable declaration
extern cv::VideoCapture cap;
extern int server_fd, client_fd;

extern dlib::rectangle biggestFaceRect; // Biggest face rectangle
extern bool hasFace; // Whether face recongition
extern std::mutex faceMutex; // Mutex for the above two variables
extern cv::Mat sharedFrame; // Frame for thread
extern std::mutex frameMutex; // Mutex for the above variable
extern std::atomic<bool> running; // Control whether thread runs

int camsetpage() {
  while (true) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) break;

    // Share the latest frame with the thread
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      frame.copyTo(sharedFrame);
    }

    // Get face rectangle
    dlib::rectangle faceRect;
    bool localHasFace;
    {
      std::lock_guard<std::mutex> lock(faceMutex);
      localHasFace = hasFace;
      faceRect = biggestFaceRect;
    }

    if (localHasFace) {
      // Draw face rectangle
      cv::rectangle(frame,
        cv::Point(faceRect.left(), faceRect.top()),
        cv::Point(faceRect.right(), faceRect.bottom()),
        cv::Scalar(0, 255, 0), 2);
    }

    // send frame to client
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    uint32_t size = buf.size();

    if (write(client_fd, &size, sizeof(size)) != sizeof(size)) break;
    if (write(client_fd, buf.data(), size) != size) break;
  }

  return 0;
}