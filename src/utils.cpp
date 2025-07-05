#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include "utils.h"

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