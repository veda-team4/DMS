#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    widgetStack = new QStackedWidget(this);
    startPage = new StartPage;
    widgetStack->addWidget(startPage);
    widgetStack->setCurrentWidget(startPage);
    setCentralWidget(widgetStack);
    // connect(startPage &StartPage::startClicked, this, &MainWindow::showMainPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMainPage() {
    //
}
