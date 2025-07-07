#ifndef MONITORPAGE_H
#define MONITORPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QByteArray>
#include "basepage.h"

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
  int expectedSize = -1;
  void readFrame();
};

#endif // MONITORPAGE_H
