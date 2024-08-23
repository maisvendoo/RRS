#include    <mainwindow.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>

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
void MainWindow::slotConnectedToSimulator()
{
    tcp_client->sendRequest(STYPE_TOPOLOGY_DATA);
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

    trainUpdateTimer->start(tcp_config.request_interval);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnUpdateTrainData()
{
    tcp_client->sendRequest(STYPE_TRAIN_POSITION);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSimulatorData(QByteArray &sim_data)
{
    train_data.deserialize(sim_data);

    ui->label->setText(QString("%1").arg(train_data.time));
    ui->label_2->setText(QString("%1").arg(train_data.vehicles[2].position_x));

}
