#ifndef CALIBRATEPAGE_H
#define CALIBRATEPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include <QPixmap>
#include <QProcess>
#include <QThread>
#include <QTimer>

namespace Ui {
  class CalibratePage;
}

class CalibratePage : public QWidget {
  Q_OBJECT

public:
  explicit CalibratePage(QWidget* parent, QLocalSocket* socket);
  ~CalibratePage();

  void activate();
  void deactivate();

private slots:
  void moveToNextStep();

signals:
  void moveToNext();

private:
  Ui::CalibratePage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  int expectedSize = -1;
  void readFrame();
  int clickCount = 0;
};

#endif // CALIBRATEPAGE_H
