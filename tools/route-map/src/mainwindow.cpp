#include    "mainwindow.h"
#include    "./ui_mainwindow.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcp_client->init("../cfg/route-map-tcp.xml");

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

    trainUpdateTimer->start(100);
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
}
