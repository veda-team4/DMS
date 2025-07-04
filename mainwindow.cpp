#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    serverProcess = new QProcess(this);
    socket = new QLocalSocket(this);

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

    ui->setupUi(this);
    widgetStack = new QStackedWidget(this);
    startPage = new StartPage;
    camSetPage = new CamSetPage(nullptr, socket);
    widgetStack->addWidget(startPage);
    widgetStack->addWidget(camSetPage);
    widgetStack->setCurrentWidget(startPage);
    setCentralWidget(widgetStack);
    connect(startPage, &StartPage::startClicked, this, &MainWindow::showCamSetPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showCamSetPage() {
    widgetStack->setCurrentWidget(camSetPage);
}
