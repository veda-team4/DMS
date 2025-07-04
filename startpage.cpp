#include "startpage.h"
#include "ui_startpage.h"

StartPage::StartPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartPage)
{
    ui->setupUi(this);
    connect(ui->nextButton, &QPushButton::clicked, this, &StartPage::nextClicked);
}

StartPage::~StartPage()
{
    delete ui;
}
