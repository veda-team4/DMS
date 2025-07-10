#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <QString>
#include <QLocalSocket>
#include <QByteArray>
#include "protocols.h"

// ************************ 편의 기능 함수 ************************
// 로그 출력 함수
void writeLog(std::string log);
// ****************************************************************

// ********************* 소켓 입출력 함수 *************************
// 버퍼로부터 주어진 길이의 바이트만큼 읽어 파일 디스크립터에 써주는 함수
int writeNBytes(QLocalSocket* socket, const void* buf, int len);
// socket에 암호화된 1바이트 명령어 전송하는 함수
void writeEncryptedCommand(QLocalSocket* socket, uint8_t command);
// ****************************************************************

// ************* OpenSSL 이용한 암호화, 복호화 함수 ***************
// aes256_cbc 방식으로 암호화 하는 함수
bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int* plaintext_len);
// aes256_cbc 방식으로 복호화 하는 함수
bool aes_encrypt(const unsigned char* plaintext, int plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext, int* ciphertext_len);
// ****************************************************************

#endif // UTILS_H
