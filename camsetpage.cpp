#include "camsetpage.h"
#include "ui_camsetpage.h"
#include "functions.h"

CamSetPage::CamSetPage(QWidget *parent, QLocalSocket* socket) :
    QWidget(parent), socket(socket),
    ui(new Ui::CamSetPage)
{
    ui->setupUi(this);
    connect(ui->nextButton, &QPushButton::clicked, this, &CamSetPage::nextClicked);
}

CamSetPage::~CamSetPage()
{
    delete ui;
}

void CamSetPage::activated() {
    sendCommand("camset", socket);
    connect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readFrame);
}

void CamSetPage::deactivated() {
    sendCommand("stop", socket);
    disconnect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readFrame);
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
