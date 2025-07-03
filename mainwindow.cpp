#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    widgetStack = new QStackedWidget(this);
    startPage = new StartPage;
    camSetPage = new CamSetPage;
    widgetStack->addWidget(startPage);
    widgetStack->addWidget(camSetPage);
    widgetStack->setCurrentWidget(startPage);
    setCentralWidget(widgetStack);
    connect(startPage, &StartPage::startClicked, this, &MainWindow::showCamSetPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showCamSetPage() {
    widgetStack->setCurrentWidget(camSetPage);
}
