#include <iostream>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "utils.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // 상단 바 제거
  this->setWindowFlags(Qt::FramelessWindowHint);

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
  widgetStack = new QStackedWidget(this);
  widgetStack->addWidget(startPage);
  widgetStack->addWidget(camSetPage);
  widgetStack->addWidget(calibratePage);
  widgetStack->addWidget(monitorPage);
  widgetStack->setCurrentWidget(startPage);

  // 메인 윈도우의 중앙 위젯을 widgetStack 으로 설정
  setCentralWidget(widgetStack);

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
  delete widgetStack;
  socket->disconnectFromServer();
  delete socket;
  serverProcess->terminate();
  if (!serverProcess->waitForFinished(3000)) {
    serverProcess->kill();
  }
}

void MainWindow::showStartPage() {
  if (auto prev = qobject_cast<BasePage*>(widgetStack->currentWidget())) {
    prev->deactivate();
  }
  widgetStack->setCurrentWidget(startPage);
}

void MainWindow::showCamSetPage() {
  if (auto prev = qobject_cast<BasePage*>(widgetStack->currentWidget())) {
    prev->deactivate();
  }
  widgetStack->setCurrentWidget(camSetPage);
  camSetPage->activate();
}

void MainWindow::showCalibratePage() {
  if (auto prev = qobject_cast<BasePage*>(widgetStack->currentWidget())) {
    prev->deactivate();
  }
  widgetStack->setCurrentWidget(calibratePage);
  calibratePage->activate();
}

void MainWindow::showMonitorPage() {
  if (auto prev = qobject_cast<BasePage*>(widgetStack->currentWidget())) {
    prev->deactivate();
  }
  widgetStack->setCurrentWidget(monitorPage);
  monitorPage->activate();
}
