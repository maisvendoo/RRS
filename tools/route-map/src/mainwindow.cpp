#include    <mainwindow.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>
#include    <QPainter>
#include    <connector.h>
#include    <QMenu>
#include    <switch.h>

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

    connect(tcp_client, &TcpClient::setSwitchState,
            this, &MainWindow::slotGetSwitchState);

    connect(tcp_client, &TcpClient::setTrajBusyState,
            this, &MainWindow::slotGetTrajBusyState);

    connect(tcp_client, &TcpClient::setSignalsData,
            this, &MainWindow::slotGetSignalsData);

    map = new MapWidget(ui->Map);
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
void MainWindow::paintEvent(QPaintEvent *event)
{
    if (traj_list == Q_NULLPTR)
    {
        return;
    }

    map->resize(ui->Map->width(), ui->Map->height());
    map->update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnectedToSimulator()
{
    tcp_client->sendRequest(STYPE_TOPOLOGY_DATA);

    ui->ptLog->appendPlainText(tr("Send request for topology loading..."));

    tcp_client->sendRequest(STYPE_SIGNALS_LIST);
    ui->ptLog->appendPlainText(tr("Send request for signals data loading..."));
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
    this->setWindowTitle(topology->getRouteName());

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

    for (auto conn : *conn_list)
    {
        SwitchLabel *sw_label = new SwitchLabel(map);
        sw_label->setText(conn->getName());
        sw_label->conn = conn;

        connect(sw_label, &SwitchLabel::popUpMenu, this, &MainWindow::slotSwitchConnectorMenu);

        map->switch_labels.insert(conn->getName(), sw_label);
    }

    ui->ptLog->appendPlainText(tr("Topology loaded successfully!"));

    QString trajectories = QString(tr("Trajectories: %1")).arg(traj_list->size());
    QString connestors = QString(tr("Connectors: %1")).arg(conn_list->size());

    ui->ptLog->appendPlainText(trajectories);
    ui->ptLog->appendPlainText(connestors);

    map->traj_list = traj_list;
    map->conn_list = conn_list;
    map->stations = topology->getStationsList();

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

    map->train_data = &train_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSwitchConnectorMenu()
{
    SwitchLabel *sw_label = dynamic_cast<SwitchLabel *>(sender());
    QString conn_name = sw_label->conn->getName();

    Switch *sw = dynamic_cast<Switch *>(sw_label->conn);
    int state_fwd = sw->getStateFwd();
    int state_bwd = sw->getStateBwd();

    if ((state_fwd == 0) && (state_bwd == 0))
        return;

    TcpClient *tc = tcp_client;

    QMenu *menu = new QMenu(this);

    if (state_fwd != 0)
    {
        QAction *action_switch_fwd = new QAction(tr("Switch forward"), this);
        action_switch_fwd->setEnabled((sw->getStateFwd() != 2) && (sw->getStateFwd() != -2));
        menu->addAction(action_switch_fwd);

        connect(action_switch_fwd, &QAction::triggered, this, [conn_name, state_fwd, state_bwd, tc]{
            tc->sendSwitchState(conn_name, -sign(state_fwd), state_bwd);
        });
    }

    if (state_bwd != 0)
    {
        QAction *action_switch_bwd = new QAction(tr("Switch backward"), this);
        action_switch_bwd->setEnabled((sw->getStateBwd() != 2) && (sw->getStateBwd() != -2));
        menu->addAction(action_switch_bwd);

        connect(action_switch_bwd, &QAction::triggered, this, [conn_name, state_fwd, state_bwd, tc]{
            tc->sendSwitchState(conn_name, state_fwd, -sign(state_bwd));
        });
    }

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSwitchState(QByteArray &sw_state)
{
    switch_state_t switch_state;
    switch_state.deserialize(sw_state);

    Switch *sw = dynamic_cast<Switch *>(conn_list->value(switch_state.name, Q_NULLPTR));

    if (sw == Q_NULLPTR)
    {
        return;
    }

    sw->setStateFwd(switch_state.state_fwd);
    sw->setStateBwd(switch_state.state_bwd);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTrajBusyState(QByteArray &busy_data)
{
    traj_busy_state_t busy_state;
    busy_state.deserialize(busy_data);

    Trajectory *traj = (traj_list->value(busy_state.name, Q_NULLPTR));

    if (traj == Q_NULLPTR)
    {
        return;
    }

    traj->setBusyState(busy_state.is_busy);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSignalsData(QByteArray &sig_data)
{
    signals_data->deserialize(sig_data);

    if (signals_data->line_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 line signals")).arg(signals_data->line_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Failed to load signals data")));
        return;
    }

    for (auto line_signal : signals_data->line_signals)
    {
        Connector *conn = conn_list->value(line_signal->getConnectorName(), Q_NULLPTR);

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        line_signal->setConnector(conn);
    }

    map->signals_data = signals_data;
}
