#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serverProcess = new QProcess(this);
    socket = new QLocalSocket(this);

    connect(serverProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = serverProcess->readAllStandardOutput();
        qDebug() << output;  // ? ?? Qt Creator Output? ??!
    });

    serverProcess->setWorkingDirectory(SERVER_PATH);
    serverProcess->start(QString(SERVER_PATH) + QString("/camera_server"), QStringList());

    if(!serverProcess->waitForStarted()) {
        qDebug() << "Failed to start server";
    }

    QThread::sleep(2);
    socket->connectToServer(SOCKET_PATH);

    if (socket->state() != QLocalSocket::ConnectedState) {
        qDebug() << "Socket connection error:" << socket->errorString();
    }

    startPage = new StartPage;
    camSetPage = new CamSetPage(nullptr, socket);
    calibratePage = new CalibratePage(nullptr, socket);

    widgetStack = new QStackedWidget(this);
    widgetStack->addWidget(startPage);
    widgetStack->addWidget(camSetPage);
    widgetStack->addWidget(calibratePage);
    widgetStack->setCurrentWidget(startPage);
    setCentralWidget(widgetStack);

    connect(startPage, &StartPage::nextClicked, this, &MainWindow::showCamSetPage);
    connect(camSetPage, &CamSetPage::nextClicked, this, &MainWindow::showCalibratePage);
}

MainWindow::~MainWindow()
{
    delete ui;
    socket->disconnectFromServer();
}

void MainWindow::showCamSetPage() {
    widgetStack->setCurrentWidget(camSetPage);
    camSetPage->activated();
}

void MainWindow::showCalibratePage() {
    widgetStack->setCurrentWidget(calibratePage);
    camSetPage->deactivated();
    calibratePage->activated();
}
