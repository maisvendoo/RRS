#include    <mainwindow.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>
#include    <QPainter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    load_config("../cfg/route-map-tcp.xml");

    tcp_client->init(tcp_config);

    connect(tcp_client, &TcpClient::connected,
            this, &MainWindow::slotConnectedToSimulator);

    connect(tcp_client, &TcpClient::disconnected,
            this, &::MainWindow::slotDisconnectedFromSimulator);

    connect(tcp_client, &TcpClient::setTopologyData,
            this, &MainWindow::slotGetTopologyData);

    connect(trainUpdateTimer, &QTimer::timeout,
            this, &MainWindow::slotOnUpdateTrainData);

    connect(tcp_client, &TcpClient::setSimulatorData,
            this, &MainWindow::slotGetSimulatorData);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::load_config(const QString &cfg_name)
{
    CfgReader cfg;

    if (!cfg.load(cfg_name))
    {
        return;
    }

    QString secName = "Client";
    cfg.getString(secName, "HostAddr", tcp_config.host_addr);

    int tmp = 0;

    if (cfg.getInt(secName, "port", tmp))
    {
        tcp_config.port = static_cast<quint16>(tmp);
    }

    cfg.getInt(secName, "ReconnectInteval", tcp_config.reconnect_interval);
    cfg.getInt(secName, "RequestInterval", tcp_config.request_interval);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint MainWindow::coord_transform(dvec3 traj_point)
{
    QPoint p;

    shift_x = 0;
    shift_y = ui->Map->height() / 2;

    p.setX(shift_x + scale * traj_point.y);
    p.setY(shift_y + scale * traj_point.x);

    return p;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *event)
{
    if (traj_list == Q_NULLPTR)
    {
        return;
    }

    for (auto traj : *traj_list)
    {
        drawTrajectory(traj);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::drawTrajectory(Trajectory *traj)
{
    QPainter painter;
    painter.begin(this);

    for (auto track : traj->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        painter.drawLine(p0, p1);
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnectedToSimulator()
{
    tcp_client->sendRequest(STYPE_TOPOLOGY_DATA);

    ui->ptLog->appendPlainText(tr("Send request for topology loading..."));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDisconnectedFromSimulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTopologyData(QByteArray &topology_data)
{
    topology->deserialize(topology_data);

    traj_list = topology->getTrajectoriesList();
    conn_list = topology->getConnectorsList();

    if ( (traj_list == Q_NULLPTR) || (conn_list == Q_NULLPTR) )
    {
        ui->ptLog->appendPlainText(tr("Toplology loading FAILED!!!"));
        return;
    }

    if (traj_list->size() == 0)
    {
        ui->ptLog->appendPlainText(tr("Trajectories list is empty"));
        return;
    }

    if (conn_list->size() == 0)
    {
        ui->ptLog->appendPlainText(tr("Connectors list is empty"));
        return;
    }

    ui->ptLog->appendPlainText(tr("Topology loaded successfully!"));

    QString trajectories = QString(tr("Trajectories: %1")).arg(traj_list->size());
    QString connestors = QString(tr("Connectors: %1")).arg(conn_list->size());

    ui->ptLog->appendPlainText(trajectories);
    ui->ptLog->appendPlainText(connestors);

    scale = static_cast<double>(ui->Map->width()) / 1000.0;

    trainUpdateTimer->start(tcp_config.request_interval);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnUpdateTrainData()
{
    tcp_client->sendRequest(STYPE_TRAIN_POSITION);
    this->update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSimulatorData(QByteArray &sim_data)
{
    train_data.deserialize(sim_data);
}
