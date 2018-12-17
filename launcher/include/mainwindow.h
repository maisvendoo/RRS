#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>

#include    "route-info.h"
#include    "train-info.h"

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
    QString         selectedTrain;

    Ui::MainWindow  *ui;

    std::vector<route_info_t>   routes_info;
    std::vector<train_info_t>   trains_info;

    void init();

    void loadRoutesList(const std::string &routesDir);

    void loadTrainsList(const std::string &trainsDir);

    void setRouteScreenShot(const QString &path);

private slots:

    void onRouteSelection();

    void onTrainSelection();
};

#endif // MAINWINDOW_H
