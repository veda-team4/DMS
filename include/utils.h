#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>

int readNBytes(int fd, void* buf, int len);
int writeNBytes(int fd, const void* buf, int len);
double computeEAR(const dlib::full_object_detection& s, int idx);
void writeLog(std::string log);

#endif // UTILS_H