#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QTimer>

#include    <tcp-client.h>
#include    <topology.h>
#include    <simulator-update-struct.h>

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

    TcpClient *tcp_client = new TcpClient(this);

    Topology *topology = new Topology;

    QTimer *trainUpdateTimer = new QTimer(this);

    tcp_simulator_update_t train_data;

private slots:

    void slotConnectedToSimulator();

    void slotDisconnectedFromSimulator();

    void slotGetTopologyData(QByteArray &topology_data);

    void slotOnUpdateTrainData();

    void slotGetSimulatorData(QByteArray &sim_data);
};

#endif // MAINWINDOW_H
