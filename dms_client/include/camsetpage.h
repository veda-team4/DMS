#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include "basepage.h"

namespace Ui {
  class CamSetPage;
}

class CamSetPage : public BasePage {
  Q_OBJECT

public:
  explicit CamSetPage(QWidget* parent, QLocalSocket* socket);
  ~CamSetPage();

public:
  void activate() override;
  void deactivate() override;

private:
  Ui::CamSetPage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  int ciphertext_len = -1;
  quint8 cmd;
  void readSocket();
};

#endif // CAMSETPAGE_H
