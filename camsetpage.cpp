#include "camsetpage.h"
#include "ui_camsetpage.h"

CamSetPage::CamSetPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CamSetPage)
{
    ui->setupUi(this);
}

CamSetPage::~CamSetPage()
{
    delete ui;
}
