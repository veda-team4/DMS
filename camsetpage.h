#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include <QPixmap>
#include <QProcess>
#include <QThread>
#include <QTimer>

#define SERVER_PATH "/home/jinhyeok/camera_server/build"
#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

namespace Ui {
class CamSetPage;
}

class CamSetPage : public QWidget
{
    Q_OBJECT

public:
    explicit CamSetPage(QWidget *parent = nullptr);
    ~CamSetPage();

private:
    void readFrame();

private:
    Ui::CamSetPage *ui;
    QLocalSocket* socket;
    QByteArray buffer;
    int expectedSize = -1;
    QProcess* serverProcess;
};

#endif // CAMSETPAGE_H
