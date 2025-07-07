#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <QString>
#include <QLocalSocket>
#include <QByteArray>

void sendCommand(std::string cmd, QLocalSocket* socket);

void writeLog(std::string log);

#endif // UTILS_H
