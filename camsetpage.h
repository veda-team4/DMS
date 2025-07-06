#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>

namespace Ui {
  class CamSetPage;
}

class CamSetPage : public QWidget {
  Q_OBJECT

public:
  explicit CamSetPage(QWidget* parent, QLocalSocket* socket);
  ~CamSetPage();

signals:
  void moveToNext();

public:
  void activate();
  void deactivate();

private:
  Ui::CamSetPage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  int expectedSize = -1;
  void readFrame();
};

#endif // CAMSETPAGE_H
