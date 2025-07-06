#include "calibratepage.h"
#include "ui_calibratepage.h"
#include "utils.h"
#include "protocols.h"

CalibratePage::CalibratePage(QWidget* parent, QLocalSocket* socket) : QWidget(parent), socket(socket), ui(new Ui::CalibratePage) {
  ui->setupUi(this);
  ui->progressBar->setValue(0);
  connect(ui->nextButton, &QPushButton::clicked, this, &CalibratePage::moveToNextStep);

  finishTimer = new QTimer(this);
  finishTimer->setInterval(100);
  connect(finishTimer, &QTimer::timeout, this, [this]() {
    ++progressStep;
    int value = progressStep * 100 / progressTotalSteps;
    ui->progressBar->setValue(value);

    if (progressStep >= progressTotalSteps) {
      finishTimer->stop();
      moveToNextStep();
    }
    });
}

CalibratePage::~CalibratePage() {
  delete ui;
  delete finishTimer;
}

void CalibratePage::activate() {
  sendCommand("calibrate", socket);
  connect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
}

void CalibratePage::deactivate() {
  sendCommand("stop", socket);
  disconnect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
}

void CalibratePage::moveToNextStep() {
  switch (clickCount) {
  case 0:
    sendCommand("opened", socket);
    ui->infoLabel->setText("뜬 눈 크기 측정중.");
    progressStep = 0;
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    finishTimer->start();
    break;
  case 1:
    sendCommand("finish", socket);
    ui->infoLabel->setText("감은 눈의 크기를 측정합니다. 준비 완료 시 버튼을 눌러주세요.");
    break;
  case 2:
    sendCommand("closed", socket);
    ui->infoLabel->setText("감은 눈 크기 측정중.");
    progressStep = 0;
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    finishTimer->start();
    break;
  case 3:
    sendCommand("finish", socket);
    emit moveToNext();
    break;
  }
  ++clickCount;
}

void CalibratePage::readFrame()
{
  buffer.append(socket->readAll());

  while (true) {
    // 아직 명령과 길이까지 못 받았으면 대기
    if (expectedSize == -1 && buffer.size() >= 5) {
      // 1바이트 명령 코드 읽기
      quint8 cmd = static_cast<quint8>(buffer[0]);

      // type이 VIDEO 가 아닌 경우 종료
      if (cmd != VIDEO) {
        qWarning("[Client] Undefined situation");
        return;
      }

      // 4바이트 길이 읽기
      expectedSize = *reinterpret_cast<const quint32*>(buffer.constData() + 1);

      // 명령 1바이트 + 길이 4바이트 제거
      buffer.remove(0, 5);
    }

    // 데이터 길이만큼 수신 완료되었을 때 처리
    if (expectedSize != -1 && buffer.size() >= expectedSize) {
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
    else {
      break;  // 아직 데이터 부족
    }
  }
}
