#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QLocalSocket>
#include <QByteArray>

void sendCommand(std::string cmd, QLocalSocket* socket);

#endif // UTILS_H
