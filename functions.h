#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <QLocalSocket>
#include <QByteArray>

void sendCommand(std::string cmd, QLocalSocket* socket);

#endif
