#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include "threads.h"

extern cv::VideoCapture cap;
extern dlib::frontal_face_detector detector;
extern int server_fd, client_fd;

int camsetpage() {
  // Shared variable declaration
  dlib::rectangle biggestFaceRect; // Biggest face rectangle
  bool hasFace = false; // Whether face recongition
  std::mutex faceMutex; // Mutex for the above two variables
  cv::Mat sharedFrame; // Frame for thread
  std::mutex frameMutex; // Mutex for the above variable
  std::atomic<bool> running = true; // Control whether thread runs

  // Run face detection thread
  std::thread faceThread(runFaceDetectionThread,
    std::ref(running), std::ref(detector), std::ref(sharedFrame),
    std::ref(frameMutex), std::ref(biggestFaceRect), std::ref(hasFace),
    std::ref(faceMutex));

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

  running = false;
  faceThread.join();

  return 0;
}