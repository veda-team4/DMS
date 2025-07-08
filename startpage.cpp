#include "startpage.h"
#include "ui_startpage.h"

StartPage::StartPage(QWidget* parent) : BasePage(parent), ui(new Ui::StartPage) {
  ui->setupUi(this);
  connect(ui->nextButton, &QPushButton::clicked, this, &StartPage::moveToNext);

  // 종료 버튼 클릭 시 앱 종료
  connect(ui->closeButton, &QPushButton::clicked, qApp, QCoreApplication::quit);
}

StartPage::~StartPage() {
  delete ui;
}
