#include "monitorpage.h"
#include "ui_monitorpage.h"
#include "utils.h"
#include "protocols.h"

MonitorPage::MonitorPage(QWidget* parent, QLocalSocket* socket) : BasePage(parent), ui(new Ui::MonitorPage), socket(socket) {
  ui->setupUi(this);
  connect(ui->previousButton, &QPushButton::clicked, this, &MonitorPage::moveToPrevious);

  // WakeUP 레이블 번쩍번쩍 기능
  wakeupTimer = new QTimer(this);
  connect(wakeupTimer, &QTimer::timeout, this, [=]() {
    if(wakeupFlashOn) {
      ui->wakeupLabel->setStyleSheet("background-color: white; color: black; font-size: 24px; font-weight: bold; border-radius: 8px; border: 2px solid black;");
    }
    else {
      ui->wakeupLabel->setStyleSheet("background-color: red; color: black; font-size: 24px; font-weight: bold; border-radius: 8px; border: 2px solid black;");
    }
    wakeupFlashOn = !wakeupFlashOn;
  });
  ui->wakeupLabel->hide();
  ui->wakeupCloseButton->hide();
  connect(ui->wakeupCloseButton, &QPushButton::clicked, this, [=]() {
    wakeupTimer->stop();
    ui->wakeupLabel->hide();
    ui->wakeupCloseButton->hide();
    wakeupFlashing = false;
  });
}

MonitorPage::~MonitorPage()
{
  delete wakeupTimer;
  delete ui;
}

void MonitorPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  writeEncryptedCommand(socket, ProtocolType::MONITOR);
}

void MonitorPage::deactivate() {
  writeEncryptedCommand(socket, ProtocolType::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
}

void MonitorPage::readFrame() {
  buffer.append(socket->readAll());

  while (true) {
    // 단계 1: IV + 길이 수신 대기
    if (ciphertext_len == -1 && buffer.size() >= 20) {
      // 16바이트 IV 읽기
      iv = buffer.left(16);
      buffer.remove(0, 16);

      // 4바이트 암호문 길이 읽기
      ciphertext_len = *reinterpret_cast<const uint32_t*>(buffer.constData());
      buffer.remove(0, 4);
    }

    // 데이터 길이만큼 수신 완료되었을 때 처리
    if (ciphertext_len != -1 && buffer.size() >= ciphertext_len) {
      QByteArray encrypted = buffer.left(ciphertext_len);
      buffer.remove(0, ciphertext_len);
      ciphertext_len = -1;

      // 복호화
      QByteArray decrypted;
      decrypted.resize(131072);
      int decrypted_len;

      bool success = aes_decrypt(
        reinterpret_cast<const unsigned char*>(encrypted.constData()), encrypted.size(),
        key, reinterpret_cast<const unsigned char*>(iv.constData()),
        reinterpret_cast<unsigned char*>(decrypted.data()), &decrypted_len
      );

      if (!success) {
        writeLog("AES decrypt failed");
        return;
      }

      // 복호화된 평문에서 명령과 길이 추출
      quint8 cmd = static_cast<quint8>(decrypted[0]);
      quint32 dataLen = *reinterpret_cast<const quint32*>(decrypted.constData() + 1);

      if (cmd == ProtocolType::FRAME) {
        QByteArray imageData = QByteArray::fromRawData(decrypted.constData() + 5, dataLen);

        QPixmap pixmap;
        if (pixmap.loadFromData(imageData, "JPG")) {
          ui->videoLabel->setPixmap(
            pixmap.scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)
          );
        }
      }
      else if (cmd == ProtocolType::EYECLOSEDRATIO) {
        double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
        ui->sleepingBar->setValue((int)(value * 100.0));

        if(!wakeupFlashing && value >= BLINK_RATIO_THRESH) {
            wakeupFlashing = true;
            ui->wakeupLabel->show();
            wakeupTimer->start(300);
            ui->wakeupCloseButton->show();
        }
      }
      else {
        writeLog("Clear protocol number " + std::to_string(cmd));
      }
    }
    else {
      break;  // 아직 데이터 부족
    }
  }
}
