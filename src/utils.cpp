#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include "utils.h"
#include "protocols.h"

// 파일 디스크립터로부터 주어진 길이의 바이트만큼 읽어 버퍼에 저장해주는 함수
int readNBytes(int fd, void* buf, int len) {
  int totalRead = 0;
  char* buffer = (char*)buf;

  while (totalRead < len) {
    int bytesRead = read(fd, buffer + totalRead, len - totalRead);

    if (bytesRead < 0) {
      if (errno == EINTR) {
        continue;  // 시그널 인터럽트 시 다시 시도
      }
      perror("read");
      return -1;     // 읽기 실패
    }
    if (bytesRead == 0) {
      // EOF 또는 연결 끊김
      break;
    }

    totalRead += bytesRead;
  }

  return totalRead == len ? totalRead : -1;
}

// 버퍼로부터 주어진 길이의 바이트만큼 읽어 파일 디스크립터에 써주는 함수
int writeNBytes(int fd, const void* buf, int len) {
  int totalWritten = 0;
  const char* buffer = (const char*)buf;

  while (totalWritten < len) {
    ssize_t bytesWritten = write(fd, buffer + totalWritten, len - totalWritten);

    if (bytesWritten < 0) {
      if (errno == EINTR) {
        continue;  // 시그널 인터럽트 시 다시 시도
      }
      perror("write");
      return -1;     // 쓰기 실패
    }

    if (bytesWritten == 0) {
      // write가 0을 반환하면 더 이상 쓸 수 없다는 뜻 → 실패
      return -1;
    }

    totalWritten += bytesWritten;
  }

  return totalWritten == len ? totalWritten : -1;
}

// 주어진 프레임 써주는 함수
int writeFrame(int fd, const std::vector<uchar>& buf) {
  uint8_t protocol = FRAME;
  uint32_t len = buf.size();
  if (writeNBytes(fd, &protocol, 1) == -1) return -1;
  if (writeNBytes(fd, &len, 4) == -1) return -1;
  if (writeNBytes(fd, buf.data(), len) == -1) return -1;
  return 0;
}

// 주어진 프로토콜과 데이터(double) 써주는 함수
int writeData(int fd, uint8_t protocol, double data) {
  uint32_t len = sizeof(data);
  if (writeNBytes(fd, &protocol, 1) == -1) return -1;
  if (writeNBytes(fd, &len, 4) == -1) return -1;
  if (writeNBytes(fd, &data, len) == -1) return -1;
  return 0;
}

double computeEAR(const dlib::full_object_detection& s, int idx) {
  auto dist = [](const dlib::point& a, const dlib::point& b) {
    double dx = a.x() - b.x(), dy = a.y() - b.y();
    return std::sqrt(dx * dx + dy * dy);
    };
  double A = dist(s.part(idx + 1), s.part(idx + 5));
  double B = dist(s.part(idx + 2), s.part(idx + 4));
  double C = dist(s.part(idx), s.part(idx + 3));
  return (A + B) / (2.0 * C);
}

void writeLog(std::string log) {
  std::cout << "[Server] " << log << std::endl;
}