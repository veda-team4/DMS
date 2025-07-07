#include "utils.h"
#include "protocols.h"

// COMMAND 프로토콜 이용하여 전송
void sendCommand(std::string cmd, QLocalSocket* socket) {
  QByteArray packet;
  uint8_t command = COMMAND;
  uint32_t length = static_cast<uint32_t>(cmd.length());

  packet.append(static_cast<char>(command));
  packet.append(reinterpret_cast<const char*>(&length), sizeof(length));
  packet.append(cmd.c_str(), static_cast<int>(cmd.length()));

  socket->write(packet);
  socket->flush();
}

int readNBytes(QLocalSocket* socket, void* buf, int len) {
  int totalRead = 0;

  char* buffer = (char*)buf;

  while (totalRead < len) {
    qint64 bytesRead = socket->read(buffer + totalRead, len - totalRead);

    if (bytesRead < 0) {
      qWarning() << "Socket read error:" << socket->errorString();
      return -1;
    }
    else if (bytesRead == 0) {
      break;
    }

    totalRead += bytesRead;
  }

  return (totalRead == len) ? totalRead : -1;
}

void writeLog(std::string log) {
  std::cout << "[Client] " << log << std::endl;
}