#include "calibratepage.h"
#include "ui_calibratepage.h"
#include "functions.h"

CalibratePage::CalibratePage(QWidget *parent, QLocalSocket* socket) :
    QWidget(parent), socket(socket),
    ui(new Ui::CalibratePage)
{
    ui->setupUi(this);
}

CalibratePage::~CalibratePage()
{
    delete ui;
}

void CalibratePage::activated() {
    sendCommand("calibrate", socket);
    connect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
}

void CalibratePage::deactivated() {
    sendCommand("stop", socket);
    disconnect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
}

void CalibratePage::readFrame()
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
