#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QTimer>

#include    <tcp-client.h>
#include    <topology.h>
#include    <simulator-update-struct.h>
#include    <map-widget.h>
#include    <switch-label.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:

    Ui::MainWindow *ui;

    tcp_config_t tcp_config;

    TcpClient *tcp_client = new TcpClient(this);

    Topology *topology = new Topology;

    QTimer *trainUpdateTimer = new QTimer(this);

    tcp_simulator_update_t train_data;

    traj_list_t *traj_list = Q_NULLPTR;

    conn_list_t *conn_list = Q_NULLPTR;

    MapWidget *map;

    void load_config(const QString &cfg_name);

    /// Преобразование координат точки траекторри в координаты виджета
    QPoint coord_transform(dvec3 traj_point);

    void paintEvent(QPaintEvent *event);

private slots:

    void slotConnectedToSimulator();

    void slotDisconnectedFromSimulator();

    void slotGetTopologyData(QByteArray &topology_data);

    void slotOnUpdateTrainData();

    void slotGetSimulatorData(QByteArray &sim_data);
};

#endif // MAINWINDOW_H
