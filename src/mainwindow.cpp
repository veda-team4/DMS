#include <iostream>
#include <QMouseEvent>
#include <QWindow>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "utils.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // 상단 바 제거
  this->setWindowFlags(Qt::FramelessWindowHint);

  // (0, 0) 으로 이동
  this->move(0, 0);

  // dragLabel에 이벤트 필터 설치
  ui->dragLabel->installEventFilter(this);

  // 종료 버튼 클릭 시 앱 종료
  connect(ui->closeButton, &QPushButton::clicked, qApp, QCoreApplication::quit);

  // 종료 버튼 꾸미기
  ui->closeButton->setStyleSheet("QPushButton { background-color: red; color: white; border: none; }");

  // 서버 프로세스, 소켓 생성
  serverProcess = new QProcess(this);
  socket = new QLocalSocket(this);

  // 서버 프로세스의 표준 출력을 Qt의 출력버퍼에 연결
  connect(serverProcess, &QProcess::readyReadStandardOutput, this, [=]() {
    QByteArray output = serverProcess->readAllStandardOutput();
    std::cout << output.toStdString();
    });

  // 서버 프로세스 실행
  serverProcess->setWorkingDirectory(SERVER_PATH);
  serverProcess->start(QString(SERVER_PATH) + QString("/camera_server"), QStringList());

  // 서버 프로세스 실행 기다림
  if (!serverProcess->waitForStarted(50000)) {
    writeLog("Failed to start server");
  }

  // 서버 소켓과 연결 (10초 동안 연결 시도)
  constexpr int MAX_RETRY_MS = 10000;
  constexpr int INTERVAL_MS = 1000;
  int elapsed = 0;
  while (elapsed < MAX_RETRY_MS) {
    socket->connectToServer(SOCKET_PATH);
    if (socket->state() == QLocalSocket::ConnectedState) {
      writeLog("Server connected.");
      break;
    }
    socket->abort();  // 실패한 연결 정리
    QThread::msleep(INTERVAL_MS);
    elapsed += INTERVAL_MS;
  }

  // 소켓 연결 실패 시 오류 처리
  if (socket->state() != QLocalSocket::ConnectedState) {
    writeLog(std::string("Socket connection error: ") + socket->errorString().toStdString());
  }

  // 페이지 생성
  startPage = new StartPage(nullptr);
  camSetPage = new CamSetPage(nullptr, socket);
  calibratePage = new CalibratePage(nullptr, socket);
  monitorPage = new MonitorPage(nullptr, socket);

  // 페이지 스택에 추가
  ui->stackedWidget->addWidget(startPage);
  ui->stackedWidget->addWidget(camSetPage);
  ui->stackedWidget->addWidget(calibratePage);
  ui->stackedWidget->addWidget(monitorPage);
  ui->stackedWidget->setCurrentWidget(startPage);

  // 각 페이지에서 버튼 클릭 시 다음 또는 이전 페이지로 이동할 수 있도록 설정
  connect(startPage, &StartPage::moveToNext, this, &MainWindow::showCamSetPage);

  connect(camSetPage, &CamSetPage::moveToNext, this, &MainWindow::showCalibratePage);
  connect(camSetPage, &CamSetPage::moveToPrevious, this, &MainWindow::showStartPage);

  connect(calibratePage, &CalibratePage::moveToNext, this, &MainWindow::showMonitorPage);
  connect(calibratePage, &CalibratePage::moveToPrevious, this, &MainWindow::showCamSetPage);

  connect(monitorPage, &MonitorPage::moveToPrevious, this, &MainWindow::showCalibratePage);
}

MainWindow::~MainWindow() {
  delete ui;
  delete startPage;
  delete camSetPage;
  delete calibratePage;
  socket->disconnectFromServer();
  delete socket;
  serverProcess->terminate();
  if (!serverProcess->waitForFinished(3000)) {
    serverProcess->kill();
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  if (obj == ui->dragLabel) {
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
        return true;
      }
    }
    else if (event->type() == QEvent::MouseMove) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (m_isDragging && (mouseEvent->buttons() & Qt::LeftButton)) {
        move(mouseEvent->globalPos() - m_dragStartPosition);
        return true;
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_isDragging = false;
        return true;
      }
    }
  }

  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showStartPage() {
  if (auto prev = qobject_cast<BasePage*>(ui->stackedWidget->currentWidget())) {
    prev->deactivate();
  }
  ui->stackedWidget->setCurrentWidget(startPage);
}

void MainWindow::showCamSetPage() {
  if (auto prev = qobject_cast<BasePage*>(ui->stackedWidget->currentWidget())) {
    prev->deactivate();
  }
  ui->stackedWidget->setCurrentWidget(camSetPage);
  camSetPage->activate();
}

void MainWindow::showCalibratePage() {
  if (auto prev = qobject_cast<BasePage*>(ui->stackedWidget->currentWidget())) {
    prev->deactivate();
  }
  ui->stackedWidget->setCurrentWidget(calibratePage);
  calibratePage->activate();
}

void MainWindow::showMonitorPage() {
  if (auto prev = qobject_cast<BasePage*>(ui->stackedWidget->currentWidget())) {
    prev->deactivate();
  }
  ui->stackedWidget->setCurrentWidget(monitorPage);
  monitorPage->activate();
}
