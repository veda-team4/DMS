#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include <QPixmap>

#define SOCKET_PATH "/home/jinhyeok/ear_qt/camera/.face_socket"

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
};

#endif // CAMSETPAGE_H
