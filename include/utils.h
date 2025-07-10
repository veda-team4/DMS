#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <vector>

int readNBytes(int fd, void* buf, int len);
int writeNBytes(int fd, const void* buf, int len);
int writeFrame(int fd, const std::vector<uchar>& buf);
int writeData(int fd, uint8_t protocol, double data);
int writeProtocol(int fd, uint8_t protocol);
uint8_t readEncryptedCommand(int fd);
uint8_t readEncryptedCommandNonBlock(int fd);
int writeEncryptedFrame(int fd, const std::vector<uchar>& buf);
int writeEncryptedData(int fd, uint8_t protocol, double data);
double computeEAR(const dlib::full_object_detection& s, int idx);
void writeLog(std::string log);
bool aes_encrypt(const unsigned char* plaintext, int plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext, int* ciphertext_len);
bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int* plaintext_len);
#endif // UTILS_H