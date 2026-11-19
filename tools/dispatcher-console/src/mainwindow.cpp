#include    <mainwindow.h>
#include    <CfgReader.h>
#include    <filesystem.h>
#include    <QApplication>
#include    <QStatusBar>
#include    <QVBoxLayout>
#include    <QTimer>
#include    <QDataStream>
#include    <QSplitter>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Dispatcher Console"));
    resize(1400, 900);

    QSplitter *sp = new QSplitter(Qt::Vertical, this);
    mnemonic = new MnemonicWidget(this);
    sp->addWidget(mnemonic);

    ptLog = new QPlainTextEdit(this);
    ptLog->setReadOnly(true); ptLog->setMaximumBlockCount(1000); ptLog->setMaximumHeight(200);
    sp->addWidget(ptLog);
    setCentralWidget(sp);

    lblStatus = new QLabel(tr("Disconnected"));
    statusBar()->addPermanentWidget(lblStatus);

    tcp_client = new TcpClient(this);

    connect(tcp_client, &TcpClient::connected, this, &MainWindow::slotConnected);
    connect(tcp_client, &TcpClient::disconnected, this, &MainWindow::slotDisconnected);
    connect(tcp_client, &TcpClient::sendLogMessage, this, &MainWindow::slotRecvLogMessage);
    connect(tcp_client, &TcpClient::setStationsData, this, &MainWindow::slotStationsData);

    connect(tcp_client, &TcpClient::setSignalsData, this, [this](QByteArray &d)
    {
        dispatcher_data_t::deserializeSignals(d, m_disp_data);
        mnemonic->setDispatcherData(&m_disp_data);
        ptLog->appendPlainText(tr("Signals updated"));
    });

    connect(tcp_client, &TcpClient::setSwitchState, this, [this](QByteArray &d)
    {
        switch_view_t sw;
        if (dispatcher_data_t::deserializeSwitch(d, sw))
            m_disp_data.switches[sw.name] = sw;
        mnemonic->update();
    });

    connect(tcp_client, &TcpClient::setTrajBusyState, this, [this](QByteArray &d)
    {
        traj_busy_view_t tb;
        if (dispatcher_data_t::deserializeTrajBusy(d, tb))
            m_disp_data.trajectory_states[tb.name] = tb;
        mnemonic->update();
    });

    connect(tcp_client, &TcpClient::setTrainInfo, this, [this](QByteArray &d)
    {
        dispatcher_data_t::deserializeTrains(d, m_disp_data.trains);
        mnemonic->update();
    });

    connect(tcp_client, &TcpClient::setVehiclesPositions, this, [this](QByteArray &d)
    {
        dispatcher_data_t::deserializePos(d, m_disp_data.pos_update);
        mnemonic->update();
    });

    loadSettingsGUI();
    loadConfig("../cfg/dispatcher-console-tcp.xml");
    QTimer::singleShot(100, tcp_client, &TcpClient::slotConnect);
}

MainWindow::~MainWindow()
{
    if (tcp_client)
    {
        disconnect(tcp_client, nullptr, this, nullptr);
        tcp_client->slotDisconnect();
    }
}

void MainWindow::loadSettingsGUI()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cp = fs.combinePath(fs.getConfigDir(), "gui-settings.xml");
    CfgReader cfg;
    if (cfg.load(QString(cp.c_str())))
    {
        QString tn = "";
        if (!cfg.getString("GUISettings", "Theme", tn)) tn = "dark-jedy";
        std::string tp = fs.combinePath(fs.getThemeDir(), tn.toStdString() + ".qss");
        QFile f(QString(tp.c_str()));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            setStyleSheet(f.readAll());
            f.close();
        }
    }
}

void MainWindow::loadConfig(const QString &fn)
{
    CfgReader cfg;
    if (!cfg.load(fn)) { ptLog->appendPlainText(tr("Config not found")); return; }
    QString sec = "Client";
    cfg.getString(sec, "HostAddr", tcp_config.host_addr);
    int tmp = 0;
    if (cfg.getInt(sec, "port", tmp)) tcp_config.port = (quint16)tmp;
    cfg.getBool(sec, "ShowServerAddr", tcp_config.show_server_addr);
    cfg.getInt(sec, "ReconnectInteval", tcp_config.reconnect_interval);
    ptLog->appendPlainText(tr("Config: %1:%2").arg(tcp_config.host_addr).arg(tcp_config.port));
}

void MainWindow::updateStatus(const QString &t) { if (lblStatus) lblStatus->setText(t); }

void MainWindow::slotConnected()
{
    ptLog->appendPlainText(tr("Connected"));
    updateStatus(tr("Connected"));
    tcp_client->sendRequest(STYPE_REQUEST_STATIONS_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_TOPOLOGY_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);
    tcp_client->sendRequest(STYPE_REQUEST_TRAINS_UPDATE);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, 0.07);
}

void MainWindow::slotDisconnected()
{
    ptLog->appendPlainText(tr("Disconnected"));
    updateStatus(tr("Disconnected"));
}

void MainWindow::slotStationsData(QByteArray &data)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    quint32 c; st >> c;
    QVector<station_info_t> sv; sv.reserve(c);
    for (quint32 i = 0; i < c; ++i)
    {
        QString n; double x, y, z; st >> n >> x >> y >> z;
        sv.append({n, x, y});
    }
    station_model.loadStations(sv);
    mnemonic->setStationModel(&station_model);
    ptLog->appendPlainText(tr("Stations: %1").arg(sv.size()));
    for (const auto &s : sv) ptLog->appendPlainText(tr("  %1").arg(s.name));
}

void MainWindow::slotRecvLogMessage(QString msg) { ptLog->appendPlainText(msg); }