#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include <QPixmap>
#include <QProcess>
#include <QThread>
#include <QTimer>

namespace Ui {
class CamSetPage;
}

class CamSetPage : public QWidget
{
    Q_OBJECT

public:
    explicit CamSetPage(QWidget *parent, QLocalSocket* socket);
    ~CamSetPage();

signals:
    void nextClicked();

private:
    void readFrame();

public:
    void activated();
    void deactivated();

private:
    Ui::CamSetPage *ui;
    QLocalSocket* socket;
    QByteArray buffer;
    int expectedSize = -1;
};

#endif // CAMSETPAGE_H
