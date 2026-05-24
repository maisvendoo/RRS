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
#include    <gpu-info.h>
#include    <CfgEditor.h>

#include    "graphsettingswindow.h"
#include    "winver.h"

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

    /// Load settings
    void loadConfig();

    /// Load GUI settings
    void loadSettingsGUI();

    /// Apply new graph settings
    //void applyGraphSettings(FieldsDataList &fd_list, Ui::MainWindow *ui);

    /// Save graph settings to file
    //void saveGraphSettings(FieldsDataList &fd_list);

    /// Генерация сценарной команды setDate
    QString createLuaSetDate(QDateEdit *dateEdit);

    /// Генерация сценарной команды setTime
    QString createLuaSetTime(QTimeEdit *timeEdit);

    /// Генерация Lua-кода установки поезда
    QStringList createLuaSetTrain(size_t idx, const active_train_t &at);

    /// Генерация кода сценария
    QStringList createTmpScenarioCode(const std::vector<active_train_t> &active_trains);

    /// Создание фалй сценария на основе ручной расстановки в лаунчере
    void createScenario(const QString &route_name,
                        const QStringList &scnCode,
                        const QString scenario_name = STARTUP_SCN_SUBDIR);

    /// Перезагрузка списка сценариев в интерфейс
    void reloadScenariosList();

    void showTrainsConfigTip();

    QLabel *trainsConfigTip = nullptr;

    void hideTrainsConfigsTip();

    std::vector<gpu_info_t> gpus_info;

    void gpuDiagnostics();

    bool start_viewer_allowed = false;

    GraphSettingsWindow *graphSettingsWindow = new GraphSettingsWindow(this);

    RequireWindowsVersion winver;

    void createHelpMenu();

    void createToolsMenu();

    std::vector<QProcess *> toolProcs;

    QMainWindow *helpWindow = new QMainWindow(this);

    void centerWindow(QWidget* window);

    /// Все-таки сохраняемые опции из данного окна
    FieldsDataList fd_options;

    static const   QString AUTO_START_VIEWER;
    static const   QString AUTO_START_ROUTE_MAP;

    void updateOptions(FieldsDataList &fd_options);

    void applyOptions(FieldsDataList &fd_options, Ui::MainWindow *ui);

    void saveOptions(FieldsDataList &fd_options);

    void closeEvent(QCloseEvent *event) override;

    const int PROC_WAIT_TIMEOUT = 1000;

    bool terminateProcess(QProcess *proc) const;

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

    void slotOnScenarioSelection(int cur_idx);

    void slotSaveTrainsConfigAsScenario();   
};


#endif // MAINWINDOW_H
