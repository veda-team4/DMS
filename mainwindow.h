#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QProcess>
#include <QLocalSocket>
#include "startpage.h"
#include "camsetpage.h"
#include "calibratepage.h"

#define SERVER_PATH "/home/jinhyeok/camera_server/build"
#define SOCKET_PATH "/home/jinhyeok/camera_server/build/.face_socket"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void showCamSetPage();
  void showCalibratePage();

private:
  QStackedWidget* widgetStack;
  StartPage* startPage;
  CamSetPage* camSetPage;
  CalibratePage* calibratePage;
  Ui::MainWindow* ui;

  QProcess* serverProcess;
  QLocalSocket* socket;
};
#endif // MAINWINDOW_H
