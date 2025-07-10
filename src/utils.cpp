#include "utils.h"
#include "protocols.h"

#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

void writeProtocol(QLocalSocket* socket, uint8_t protocol) {
  QByteArray packet;
  packet.append(static_cast<char>(protocol));
  socket->write(packet);
  socket->flush();
}

// 버퍼로부터 주어진 길이의 바이트만큼 읽어 파일 디스크립터에 써주는 함수
int writeNBytes(QLocalSocket* socket, const void* buf, int len) {
  int totalWritten = 0;
  const char* buffer = (const char*)buf;

  while (totalWritten < len) {
    ssize_t bytesWritten = socket->write(buffer + totalWritten, len - totalWritten);

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

void writeEncryptedCommand(QLocalSocket* socket, uint8_t command) {
  unsigned char iv[16];
  RAND_bytes(iv, sizeof(iv));  // 무작위 IV 생성

  // 평문 명령 1바이트 준비
  unsigned char plaintext[1] = { command };

  unsigned char ciphertext[32];  // AES 패딩 때문에 16 이상 필요
  int ciphertext_len;

  aes_encrypt(plaintext, 1, key, iv, ciphertext, ciphertext_len);

  // 1. IV 전송
  writeNBytes(socket, iv, 16);

  // 2. 암호문 길이 전송
  writeNBytes(socket, &ciphertext_len, 4);

  // 3. 암호문 전송
  writeNBytes(socket, ciphertext, ciphertext_len);
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