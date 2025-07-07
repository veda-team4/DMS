#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <QString>
#include <QLocalSocket>
#include <QByteArray>

void sendCommand(std::string cmd, QLocalSocket* socket);

int readNBytes(QLocalSocket* socket, void* buf, int len);

void writeLog(std::string log);

#endif // UTILS_H
