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

#include    <route-info.h>
#include    <train-info.h>
#include    <waypoint.h>
#include    <active-train.h>
#include    <CfgEditor.h>

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

    /// Selected route directory name
    QString         selectedRouteDirName;
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

    /// Viewer settings
    FieldsDataList  fd_list;

    static const   QString WIDTH;
    static const   QString HEIGHT;
    static const   QString FULLSCREEN;
    static const   QString FOV_Y;
    static const   QString ZNEAR;
    static const   QString ZFAR;
    static const   QString SCREEN_NUM;
    static const   QString WIN_DECOR;
    static const   QString DOUBLE_BUFF;
    static const   QString NOTIFY_LEVEL;
    static const   QString VIEW_DIST;

    QString settings_path;

    std::vector<train_position_t> fwd_train_positions;

    std::vector<train_position_t> bwd_train_positions;

    train_position_t selected_train_position;

    std::vector<active_train_t> active_trains;

    /// Launcer initialization
    void init();

    /// Loading of routes list
    void loadRoutesList(const std::string &routesDir);

    /// Loading of trains list
    void loadTrainsList(const std::string &trainsDir);

    /// Start simulation
    void startSimulator();

    /// Start viewer
    void startViewer();

    /// Load theme
    void loadTheme();

    /// Load graphics settings
    void loadGraphicsSettings(QString file_name);

    /// Update graphics settings
    void updateGraphSettings(FieldsDataList &fd_list, Ui::MainWindow *ui);

    /// Apply new graph settings
    void applyGraphSettings(FieldsDataList &fd_list, Ui::MainWindow *ui);

    /// Save graph settings to file
    void saveGraphSettings(FieldsDataList &fd_list);

    void loadTrainPositions(const QString &routeDir);

    int getSelectedActiveTrainIndex();

    void updateActiveTrains();

private slots:

    void onRouteSelection();

    void onTrainSelection();

    void onStartPressed();

    void onSimulatorStarted();

    void onSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onViewerFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onStationSelected(int index);

    void slotChangedGraphSetting(int);

    void slotChangedGraphSetting(double);

    void slotCancelGraphSettings();

    void slotApplyGraphSettings();

    void slotAddActiveTrain();

    void slotDeleteActiveTrain();

    void slotActiveTrainCellChanged(int row, int column);

    void slotActiveTrainDirectionChange(int idx);

    void slotActiveTrainTrajectoryChange(int idx);

    void slotTrainCoordValueChanged(double value);
};


#endif // MAINWINDOW_H
