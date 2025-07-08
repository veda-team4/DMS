#include "utils.h"
#include "protocols.h"

void writeProtocol(QLocalSocket* socket, uint8_t protocol) {
  QByteArray packet;
  packet.append(static_cast<char>(protocol));
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