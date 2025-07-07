#ifndef MONITORPAGE_H
#define MONITORPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QByteArray>

namespace Ui {
  class MonitorPage;
}

class MonitorPage : public QWidget {
  Q_OBJECT

public:
  explicit MonitorPage(QWidget* parent, QLocalSocket* socket);
  ~MonitorPage();

  void activate();
  void deactivate();

private:
  Ui::MonitorPage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  int expectedSize = -1;
  void readFrame();
};

#endif // MONITORPAGE_H
