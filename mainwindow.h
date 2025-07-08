#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QProcess>
#include <QLocalSocket>
#include <QMouseEvent>
#include <QPoint>
#include "startpage.h"
#include "camsetpage.h"
#include "calibratepage.h"
#include "monitorpage.h"
#include "basepage.h"

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

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void showStartPage();
  void showCamSetPage();
  void showCalibratePage();
  void showMonitorPage();

private:
  Ui::MainWindow* ui;

  QPoint m_dragStartPosition;
  bool m_isDragging;

  BasePage* startPage;
  BasePage* camSetPage;
  BasePage* calibratePage;
  BasePage* monitorPage;

  QProcess* serverProcess;
  QLocalSocket* socket;

};
#endif // MAINWINDOW_H
