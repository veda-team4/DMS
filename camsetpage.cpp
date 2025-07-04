#include "camsetpage.h"
#include "ui_camsetpage.h"

CamSetPage::CamSetPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CamSetPage)
{
    ui->setupUi(this);

    serverProcess = new QProcess(this);
    socket = new QLocalSocket(this);

    serverProcess->setWorkingDirectory(SERVER_PATH);
    serverProcess->start(QString(SERVER_PATH) + QString("/camera_server"), QStringList());

    if(!serverProcess->waitForStarted()) {
        qDebug() << "Failed to start server";
    }

    QThread::sleep(1);
    socket->connectToServer(SOCKET_PATH);

    if (socket->state() != QLocalSocket::ConnectedState) {
        qDebug() << "Socket connection error:" << socket->errorString();
    }

    connect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readFrame);
}

CamSetPage::~CamSetPage()
{
    if(socket->isOpen()) {
        socket->disconnectFromServer();
    }
    delete ui;
}

void CamSetPage::readFrame()
{
    buffer.append(socket->readAll());

    while (true) {
        if (expectedSize == -1 && buffer.size() >= 4) {
            expectedSize = *reinterpret_cast<const quint32*>(buffer.constData());
            buffer.remove(0, 4);
        }

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
        } else {
            break;
        }
    }
}
