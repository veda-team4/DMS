#include "camsetpage.h"
#include "ui_camsetpage.h"

CamSetPage::CamSetPage(QWidget *parent, QLocalSocket* socket) :
    QWidget(parent), socket(socket),
    ui(new Ui::CamSetPage)
{
    ui->setupUi(this);
}

CamSetPage::~CamSetPage()
{
    delete ui;
}

void CamSetPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    socket->write("camsetpage\n");
    socket->flush();
    connect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readFrame);
}

void CamSetPage::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
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
