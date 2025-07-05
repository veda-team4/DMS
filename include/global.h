#ifndef GLOBAL_H
#define GLOBAL_H

extern cv::VideoCapture cap;

extern dlib::shape_predictor sp;
extern dlib::rectangle biggestFaceRect;
extern bool hasFace;
extern std::mutex faceMutex;
extern cv::Mat sharedFrame;
extern std::mutex frameMutex;
extern std::atomic<bool> running;

extern int server_fd, client_fd;

#endif // GLOBAL_H