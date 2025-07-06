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
  QTimer* finishTimer;
  int expectedSize = -1;
  void readFrame();
  int clickCount = 0;
  int progressStep = 0;
  const int progressTotalSteps = 20;
};

#endif // CALIBRATEPAGE_H
