#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QProcess>

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

    QProcess        simulatorProc;
    QProcess        viewerProc;

    void init();

    void loadRoutesList(const std::string &routesDir);

    void loadTrainsList(const std::string &trainsDir);

    void setRouteScreenShot(const QString &path);

    void startSimulator();

    void startViewer();

private slots:

    void onRouteSelection();

    void onTrainSelection();

    void onStartPressed();

    void onSimulatorStarted();

    void onSimulatorFinished(int exitCode);

    void onViewerFinished(int exitCode);
};

#endif // MAINWINDOW_H
