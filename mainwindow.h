#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "startpage.h"
#include "camsetpage.h"

#define SERVER_PATH "/home/jinhyeok/camera_server/build"
#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void showCamSetPage();
private:
    QStackedWidget* widgetStack;
    StartPage* startPage;
    CamSetPage* camSetPage;
    Ui::MainWindow *ui;

    QProcess* serverProcess;
    QLocalSocket* socket;
};
#endif // MAINWINDOW_H
