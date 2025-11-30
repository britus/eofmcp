// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
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
