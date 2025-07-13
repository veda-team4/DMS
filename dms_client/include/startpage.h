#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include "basepage.h"

namespace Ui {
  class StartPage;
}

class StartPage : public BasePage {
  Q_OBJECT
public:
  explicit StartPage(QWidget* parent, QLocalSocket* socket);
  ~StartPage();

public:
  void activate() override;
  void deactivate() override;

private:
  Ui::StartPage* ui;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  int ciphertext_len = -1;
  quint8 cmd;
  void readSocket();
};

#endif // STARTPAGE_H
