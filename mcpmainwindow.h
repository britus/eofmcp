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
