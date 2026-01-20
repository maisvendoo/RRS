#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QTimer>

#include    "command-line.h"
#include    <tcp-client.h>
#include    <topology.h>
#include    <simulator-info-struct.h>
#include    <simulator-update-struct.h>
#include    <map-widget.h>
#include    <switch-label.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class QTreeWidgetItem;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_map_command_line_t
{
    /// Train configuration file name
    option_t<QString>   host_addr;
    /// Route directory
    option_t<quint16>   port;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(route_map_command_line_t &cmd_line, QWidget *parent = nullptr);

    ~MainWindow();

private:

    Ui::MainWindow *ui;

    tcp_config_t tcp_config;

    int vehicles_pos_update_interval = 70;

    int players_update_interval = 70;

    TcpClient *tcp_client = new TcpClient(this);

    Topology *topology = new Topology;

    simulator_update_players_t players_data;

    simulator_update_pos_t train_data;

    std::vector<double> vehicles_half_length;

    MapWidget *map;

    signals_data_t *signals_data = new signals_data_t();

    void load_config(const QString &cfg_name);

    void overrideByCommandLine(route_map_command_line_t &cmd_line);

    void paintEvent(QPaintEvent *event);

    void updateStations();

    void updatePlayers();

private slots:

    void slotConnectedToSimulator();

    void slotDisconnectedFromSimulator();

    void slotGetVehicleInfoData(QByteArray &data);

    void slotGetTopologyData(QByteArray &topology_data);

    void slotGetSignalsData(QByteArray &sig_data);

    void slotGetPlayersData(QByteArray &players_update);

    void slotGetVehiclePosData(QByteArray &sim_data);

    void slotNearestTrajectoryMenu(Trajectory* nearest_traj);

    void slotSwitchConnectorMenu();

    void slotSignalControlMenu();

    void slotGetSwitchState(QByteArray &sw_state);

    void slotGetTrajBusyState(QByteArray &busy_state);

    void slotUpdateSignal(QByteArray signal_data);

    void slotRecvLogMessage(QString msg);

    void slotSetShowTrajStatus(bool is_show);

    void slotGetTrainsInfo(QByteArray& data);

    void slotRenameTrainMenu();
};

#endif // MAINWINDOW_H
