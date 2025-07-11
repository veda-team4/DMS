#ifndef THREADS_H
#define THREADS_H

#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <iostream>
#include <vector>

void runFaceDetectionThread();

#endif // THREADS_H