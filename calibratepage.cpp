#include "calibratepage.h"
#include "ui_calibratepage.h"

CalibratePage::CalibratePage(QWidget *parent, QLocalSocket* socket) :
    QWidget(parent), socket(socket),
    ui(new Ui::CalibratePage)
{
    ui->setupUi(this);
}

CalibratePage::~CalibratePage()
{
    delete ui;
}
