#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>

#include    "route-info.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace Ui
{
    class MainWindow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    QString         selectedRoutePath;

    Ui::MainWindow  *ui;

    std::vector<route_info_t>   routes_info;

    void init();

    void loadRoutesList(const std::string &routesDir);

    void setRouteScreenShot(const QString &path);

private slots:

    void onRouteSelection();
};

#endif // MAINWINDOW_H
