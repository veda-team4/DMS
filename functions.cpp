#include "functions.h"

void sendCommand(std::string cmd, QLocalSocket* socket) {
    QByteArray packet;
    packet.append(static_cast<char>(cmd.length()));
    packet.append(cmd.c_str(), static_cast<int>(cmd.length()));

    socket->write(packet);
    socket->flush();
}
