#ifndef MONITORPAGE_H
#define MONITORPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QByteArray>
#include <QTimer>
#include "basepage.h"

#define BLINK_RATIO_THRESH 0.6 // 2초 중 얼만큼을 눈을 감아야 경고할지에 대한 비율

namespace Ui {
  class MonitorPage;
}

class MonitorPage : public BasePage {
  Q_OBJECT

public:
  explicit MonitorPage(QWidget* parent, QLocalSocket* socket);
  ~MonitorPage();

  void activate() override;
  void deactivate() override;

private:
  Ui::MonitorPage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  int ciphertext_len = -1;
  quint8 cmd;

  QTimer* wakeupTimer;
  bool wakeupFlashOn = false;
  bool wakeupFlashing = false;

  void readFrame();
};

#endif // MONITORPAGE_H
