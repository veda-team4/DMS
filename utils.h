#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <QString>
#include <QLocalSocket>
#include <QByteArray>
#include "protocols.h"

void writeProtocol(QLocalSocket* socket, uint8_t protocol);

int readNBytes(QLocalSocket* socket, void* buf, int len);

void writeLog(std::string log);

#endif // UTILS_H
