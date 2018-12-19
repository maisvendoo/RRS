//------------------------------------------------------------------------------
//
//      RSS launcher main window
//      (c) maisvendoo, 17/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief RSS launcher main window
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 17/12/2018
 */

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

/*!
 * \class
 * \brief Main window class
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /// Constructor
    explicit MainWindow(QWidget *parent = nullptr);

    /// Destructor
    ~MainWindow();

private:

    /// Selected route path
    QString         selectedRoutePath;
    /// SElected train config
    QString         selectedTrain;

    Ui::MainWindow  *ui;

    /// Info about installed routes
    std::vector<route_info_t>   routes_info;
    /// Info about installed trains
    std::vector<train_info_t>   trains_info;

    /// Simulation process
    QProcess        simulatorProc;
    /// Visaulization process
    QProcess        viewerProc;

    /// Launcer initialization
    void init();

    /// Loading of routes list
    void loadRoutesList(const std::string &routesDir);

    /// Loading of trains list
    void loadTrainsList(const std::string &trainsDir);

    /// Set route shotcut
    void setRouteScreenShot(const QString &path);

    /// Start simulation
    void startSimulator();

    /// Start viewer
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
