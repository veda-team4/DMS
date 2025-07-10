#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include "utils.h"
#include "protocols.h"
#include "global.h"

static unsigned char key[33] = "abcdefghijklmnopqrstuvwxyz012345";
static unsigned char iv[16];

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
  uint8_t protocol = ProtocolType::FRAME;
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

// 주어진 프로토콜 써주는 함수
int writeProtocol(int fd, uint8_t protocol) {
  return writeNBytes(fd, &protocol, sizeof(protocol));
}

uint8_t readEncryptedCommand(int fd) {
  recv(fd, iv, 16, MSG_WAITALL); // IV 수신

  uint32_t ciphertext_len;
  recv(fd, &ciphertext_len, 4, MSG_WAITALL); // 길이 수신

  unsigned char ciphertext[32];
  recv(fd, ciphertext, ciphertext_len, MSG_WAITALL); // 암호문 수신

  unsigned char plaintext[32];
  int plaintext_len;

  aes_decrypt(ciphertext, ciphertext_len, key, iv, plaintext, plaintext_len);
  ciphertext[plaintext_len] = '\0';

  return plaintext[0];
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

bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len,
  const unsigned char* key, const unsigned char* iv,
  unsigned char* plaintext, int& plaintext_len) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return false;

  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len;
  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return true;
}

bool aes_encrypt(const unsigned char* plaintext, int plaintext_len,
  const unsigned char* key, const unsigned char* iv,
  unsigned char* ciphertext, int& ciphertext_len) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return false;

  // 초기화 (AES-256-CBC, key/iv 설정)
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len;

  // 평문 암호화
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  ciphertext_len = len;

  // 패딩 처리 및 마무리
  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return true;
}