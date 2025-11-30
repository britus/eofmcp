// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MCPMainWindow;
}
QT_END_NAMESPACE

class MCPMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MCPMainWindow(QWidget *parent = nullptr);
    ~MCPMainWindow();

private:
    Ui::MCPMainWindow *ui;
};
