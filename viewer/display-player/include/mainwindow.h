#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QWidget>

#include    "display.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QWidget
{
public:

    MainWindow(QString& module_path, QString &config_path, QWidget* parent = nullptr);

    ~MainWindow() = default;

private:

    AbstractDisplay* display = nullptr;
};

#endif // MAINWINDOW_H
