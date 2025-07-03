#include "startpage.h"
#include "ui_startpage.h"

StartPage::StartPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartPage)
{
    ui->setupUi(this);
    connect(ui->startButton, &QPushButton::clicked, this, &StartPage::startClicked);
}

StartPage::~StartPage()
{
    delete ui;
}
