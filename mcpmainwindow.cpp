#include "mcpmainwindow.h"
#include "ui_mcpmainwindow.h"

MCPMainWindow::MCPMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MCPMainWindow)
{
    ui->setupUi(this);
}

MCPMainWindow::~MCPMainWindow()
{
    delete ui;
}
