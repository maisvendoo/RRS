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

    MainWindow(QString module_path, QWidget *parent = Q_NULLPTR);

    ~MainWindow();

private:

    AbstractDisplay *display;
};

#endif // MAINWINDOW_H
