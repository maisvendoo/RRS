#include    <mainwindow.h>
#include    <CfgReader.h>
#include    <filesystem.h>
#include    <QApplication>
#include    <QStatusBar>
#include    <QVBoxLayout>
#include    <QTimer>
#include    <QDataStream>
#include    <QSplitter>
#include    <QMessageBox>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Dispatcher Console"));
    resize(1400, 900);

    // Меню станций
    mStations = menuBar()->addMenu(tr("Stations"));

    // Панель инструментов
    QToolBar *tb = addToolBar(tr("Tools"));
    actFollowTrain = tb->addAction(tr("Follow train"));
    actFollowTrain->setCheckable(true);
    actFollowTrain->setChecked(false);

    QSplitter *sp = new QSplitter(Qt::Vertical, this);
    mnemonic = new MnemonicWidget(this);
    sp->addWidget(mnemonic);

    ptLog = new QPlainTextEdit(this);
    ptLog->setReadOnly(true); ptLog->setMaximumBlockCount(1000); ptLog->setMaximumHeight(150);
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
        {
            m_disp_data.switches[sw.name] = sw;
            ptLog->appendPlainText(tr("Switch %1: fwd=%2 bwd=%3").arg(sw.name).arg(sw.state_fwd).arg(sw.state_bwd));
        }
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
        if (actFollowTrain->isChecked() && follow_vehicle >= 0 &&
            follow_vehicle < m_disp_data.pos_update.vehicles.size())
        {
            const auto &v = m_disp_data.pos_update.vehicles[follow_vehicle];
            if (!station_model.stations().empty())
            {
                // Центрируем на поезде
                const auto &ss = station_model.stations();
                int ni = 0; double md = 1e9;
                for (int i = 0; i < ss.size(); ++i)
                {
                    double dd = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
                    if (dd < md) { md = dd; ni = i; }
                }
                double target_off = -(ni * 300.0 * mnemonic->scale())
                                   + (v.pos_x - ss[ni].pos_x) * 0.5 * mnemonic->scale();
                mnemonic->setOffset(target_off);
            }
        }
        mnemonic->update();
    });

    // Управление: контекстное меню сигналов
    connect(mnemonic, &MnemonicWidget::signalRightClicked, this, [this](signal_hit_t hit)
    {
        const signal_view_t *sig = nullptr;
        if (hit.signal_type == 0 && hit.signal_index < m_disp_data.line_signals.size())
            sig = &m_disp_data.line_signals[hit.signal_index];
        else if (hit.signal_type == 1 && hit.signal_index < m_disp_data.enter_signals.size())
            sig = &m_disp_data.enter_signals[hit.signal_index];
        else if (hit.signal_type == 2 && hit.signal_index < m_disp_data.route_signals.size())
            sig = &m_disp_data.route_signals[hit.signal_index];
        else if (hit.signal_type == 3 && hit.signal_index < m_disp_data.exit_signals.size())
            sig = &m_disp_data.exit_signals[hit.signal_index];
        else if (hit.signal_type == 4 && hit.signal_index < m_disp_data.shunt_signals.size())
            sig = &m_disp_data.shunt_signals[hit.signal_index];

        if (!sig) return;

        QMenu menu(this);
        QAction *open = menu.addAction(tr("Open"));
        QAction *close = menu.addAction(tr("Close"));
        QAction *chosen = menu.exec(QCursor::pos());
        if (!chosen) return;

        // Формируем команду сигналу
        QByteArray cmd_data;
        QDataStream cmd_st(&cmd_data, QIODevice::WriteOnly);
        cmd_st.setByteOrder(QDataStream::BigEndian);
        cmd_st.setVersion(QDataStream::Qt_4_0);
        cmd_st << sig->conn_name << sig->signal_dir
               << (chosen == open)  // open_train
               << false            // open_shunting
               << false            // open_call
               << (chosen == close); // close
        tcp_client->sendSignalCommand(cmd_data);
        ptLog->appendPlainText(tr("Signal %1: %2").arg(sig->letter).arg(chosen == open ? tr("open") : tr("close")));
    });

    connect(mnemonic, &MnemonicWidget::trajectoryRightClicked, this, [this](const QString &)
    {
        QMenu menu(this);
        QAction *center = menu.addAction(tr("Center on first station"));
        if (menu.exec(QCursor::pos()) == center)
            mnemonic->setOffset(0);
    });

    // Слежение за поездом
    connect(actFollowTrain, &QAction::toggled, this, [this](bool checked)
    {
        if (checked && !m_disp_data.pos_update.vehicles.empty())
            follow_vehicle = 0;
        else
            follow_vehicle = -1;
        ptLog->appendPlainText(checked ? tr("Follow mode on") : tr("Follow mode off"));
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
    for (const auto &s : sv)
    {
        ptLog->appendPlainText(tr("  %1").arg(s.name));
        QAction *a = mStations->addAction(s.name);
        connect(a, &QAction::triggered, this, [this, &s]{
            for (int i = 0; i < station_model.stations().size(); ++i)
            {
                if (station_model.stations()[i].name == s.name)
                {
                    mnemonic->setOffset(-i * 300.0 * mnemonic->scale());
                    break;
                }
            }
        });
    }
}

void MainWindow::slotRecvLogMessage(QString msg) { ptLog->appendPlainText(msg); }