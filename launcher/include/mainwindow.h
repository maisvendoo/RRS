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
#include    <QIcon>
#include    <QToolBox>
#include    <QProcess>
#include    <QTimer>
#include    <QDateEdit>
#include    <QTimeEdit>

#include    <train-info.h>
#include    <route-info.h>
#include    <active-train.h>
#include    <server_info.h>
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
    QToolBox *tbActiveTrains;
    int selected_route_idx = -1;

    int selected_scenario_idx = -1;

    /// Info about installed trains
    std::vector<train_info_t>   trains_info;
    /// Info about installed routes
    std::vector<route_info_t>   routes_info;

    /// Info about trajectories in current selected route
    std::vector<trajectory_info_t>  *trajectrories;
    /// Info about waypoints in current selected route
    std::vector<train_position_t>   *fwd_train_positions;
    /// Info about waypoints in current selected route
    std::vector<train_position_t>   *bwd_train_positions;

    std::vector<active_train_t> active_trains;

    /// Simulation process
    QProcess        simulatorProc;
    /// Visaulization process
    QProcess        viewerProc;
    /// Dispatcher map process
    QProcess        mapProc;

    bool is_start_button_to_stop_server;
    int new_added_start_config_idx = -1;

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
    static const   QString VSYNC;
    static const   QString NOTIFY_LEVEL;
    static const   QString VIEW_DIST;
    static const   QString STARTUP_SCN_SUBDIR;

    QString settings_path;
    QString saved_servers_path;

    QMap<QString, server_info_t> saved_servers;

    QIcon icon_ok = QIcon(QString(":/images/images/1_ok.png"));
    QIcon icon_cancel = QIcon(QString(":/images/images/2_cancel.png"));
    QIcon icon_warn = QIcon(QString(":/images/images/3_warn.png"));

    QTimer update_datetime_timer;

    /// Launcer initialization
    void init();

    /// Loading of routes list
    void loadRoutesList(const std::string &routesDir);

    /// Loading of trains list
    void loadTrainsList(const std::string &trainsDir);

    /// Loading of servers list
    void loadServersList(const std::string &cfgDir);

    /// Loading of trajectories list at current selected route
    void loadTrajectories(route_info_t &route_info);

    /// Loading of waypoints list at current selected route
    void loadTrainPositions(route_info_t &route_info);

    /// Loading of waypoints list at current selected route
    void loadScenarios(route_info_t &route_info);

    /// Load scenario description
    QString loadScenarioDescription(QString path);

    /// Clear all trains and their waypoints
    void clearActiveTrainsList();

    /// Load route's last list with trains and their waypoints
    void loadActiveTrainsList();

    /// Save servers list
    void saveServersList();

    /// Start simulation
    void startSimulator();

    /// Start viewer
    void startViewer(bool local = true);

    /// Start dispatcher map
    void startMap(bool local = true);

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

    /// Генерация сценарной команды setDate
    QString createLuaSetDate(QDateEdit *dateEdit);

    /// Генерация сценарной команды setTime
    QString createLuaSetTime(QTimeEdit *timeEdit);

    /// Генерация Lua-кода установки поезда
    QStringList createLuaSetTrain(size_t idx, const active_train_t &at);

    /// Генерация кода сценария
    QStringList createTmpScenarioCode(const std::vector<active_train_t> &active_trains);

    /// Создание временного сценария
    void createTmpScenario(const QString &route_name,
                           const std::vector<active_train_t> &active_trains);

private slots:

    void slotRouteSelection();

    void slotTrainSelection();

    void slotAddActiveTrain();

    void slotDeleteActiveTrain();

    void slotSelectSavedStartConfig(int idx);

    void slotTrainConfigChanged();

    void slotUpdateActiveTrains(bool reset_start_config = true);

    void slotStartDateManuallyChanged();

    void slotStartTimeManuallyChanged();

    void slotUpdateDateTime();

    void slotStartServerPressed();

    void slotStartViewerPressed();

    void slotStartMapPressed();

    void slotSimulatorStarted();

    void slotViewerStarted();

    void slotMapStarted();

    void slotConnectViewerPressed();

    void slotConnectMapPressed();

    void slotSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void slotViewerFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void slotMapFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void slotAdditionalProcFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void slotSelectSavedServer(int idx);

    void slotChangedServerSettings();

    void slotSaveServer();

    void slotChangedGraphSetting(int);

    void slotChangedGraphSetting(double);

    void slotCancelGraphSettings();

    void slotApplyGraphSettings();

    void slotOnScenarioSelection(int cur_idx);
};


#endif // MAINWINDOW_H
