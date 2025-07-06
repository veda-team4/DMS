#include <iostream>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

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
  if (!serverProcess->waitForStarted()) {
    qDebug() << "Failed to start server";
  }
  QThread::sleep(2); // 서버 프로세스에서의 소켓 생성 시간을 고려하여 2초간 sleep

  // 서버 소켓과 연결
  socket->connectToServer(SOCKET_PATH);

  // 소켓 연결 실패 시 오류 처리
  if (socket->state() != QLocalSocket::ConnectedState) {
    qDebug() << "Socket connection error: " << socket->errorString();
  }

  // 페이지 생성
  startPage = new StartPage;
  camSetPage = new CamSetPage(nullptr, socket);
  calibratePage = new CalibratePage(nullptr, socket);

  // 페이지 스택에 추가
  widgetStack = new QStackedWidget(this);
  widgetStack->addWidget(startPage);
  widgetStack->addWidget(camSetPage);
  widgetStack->addWidget(calibratePage);
  widgetStack->setCurrentWidget(startPage);

  // 메인 윈도우의 중앙 위젯을 widgetStack 으로 설정
  setCentralWidget(widgetStack);

  // 각 페이지에서 버튼 클릭 시 다음 페이지로 이동할 수 있도록 설정
  connect(startPage, &StartPage::moveToNext, this, &MainWindow::showCamSetPage);
  connect(camSetPage, &CamSetPage::moveToNext, this, &MainWindow::showCalibratePage);
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

void MainWindow::showCamSetPage() {
  widgetStack->setCurrentWidget(camSetPage);
  camSetPage->activate();
}

void MainWindow::showCalibratePage() {
  widgetStack->setCurrentWidget(calibratePage);
  camSetPage->deactivate();
  calibratePage->activate();
}
