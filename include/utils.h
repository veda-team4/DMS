#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <QString>
#include <QLocalSocket>
#include <QByteArray>
#include "protocols.h"

void writeProtocol(QLocalSocket* socket, uint8_t protocol);
void writeEncryptedCommand(QLocalSocket* socket, uint8_t command);
int readNBytes(QLocalSocket* socket, void* buf, int len);
void writeLog(std::string log);
bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int& plaintext_len);
bool aes_encrypt(const unsigned char* plaintext, int plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext, int& ciphertext_len);

#endif // UTILS_H
