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
  expectedSize = -1;
}

void MonitorPage::readFrame() {
  buffer.append(socket->readAll());

  while (true) {
    // 아직 명령과 길이까지 못 받았으면 대기
    if (expectedSize == -1 && buffer.size() >= 5) {
      // 1바이트 명령 코드 읽기
      cmd = static_cast<quint8>(buffer[0]);

      // 4바이트 길이 읽기
      expectedSize = *reinterpret_cast<const quint32*>(buffer.constData() + 1);

      // 명령 1바이트 + 길이 4바이트 제거
      buffer.remove(0, 5);
    }

    // 데이터 길이만큼 수신 완료되었을 때 처리
    if (expectedSize != -1 && buffer.size() >= expectedSize) {
      if (cmd == ProtocolType::FRAME) {
        QByteArray imageData = buffer.left(expectedSize);
        buffer.remove(0, expectedSize);
        expectedSize = -1;

        QPixmap pixmap;
        if (pixmap.loadFromData(imageData, "JPG")) {
          ui->videoLabel->setPixmap(
            pixmap.scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)
          );
        }
      }
      else if (cmd == ProtocolType::EYECLOSEDRATIO) {
        QByteArray data = buffer.left(expectedSize);
        buffer.remove(0, expectedSize);
        expectedSize = -1;

        double value = *reinterpret_cast<const double*>(data.constData());
        ui->sleepingBar->setValue((int)(value * 100.0));

        if(!wakeupFlashing && value >= BLINK_RATIO_THRESH) {
            wakeupFlashing = true;
            ui->wakeupLabel->show();
            wakeupTimer->start(300);
            ui->wakeupCloseButton->show();
        }
      }
    }
    else {
      break;  // 아직 데이터 부족
    }
  }
}
