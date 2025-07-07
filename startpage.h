#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>
#include "basepage.h"

namespace Ui {
  class StartPage;
}

class StartPage : public BasePage {
  Q_OBJECT
public:
  explicit StartPage(QWidget* parent);
  ~StartPage();

private:
  Ui::StartPage* ui;
};

#endif // STARTPAGE_H
