#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include "basepage.h"
#include "mainwindow.h"

extern bool gestureLock;

namespace Ui {
  class CamSetPage;
}

class MainWindow;

class CamSetPage : public BasePage {
  Q_OBJECT

public:
  explicit CamSetPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket);
  ~CamSetPage();

public:
  void activate() override;
  void deactivate() override;

private:
  Ui::CamSetPage* ui;
  MainWindow* mainWindow;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  int ciphertext_len = -1;
  quint8 cmd;
  void readSocket();
};

#endif // CAMSETPAGE_H
