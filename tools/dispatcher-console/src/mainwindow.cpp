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
#include    <QInputDialog>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Dispatcher Console"));
    resize(1400, 900);

    mStations = menuBar()->addMenu(tr("Stations"));
    menuBar()->addAction(tr("Center all"), this, [this]{ mnemonic->setOffset(0); });
    menuBar()->addAction(tr("Zoom 100%"), this, [this]{ mnemonic->setScale(1.0); });

    QToolBar *tb = addToolBar(tr("Tools"));
    actFollowTrain = tb->addAction(tr("Follow"));
    actFollowTrain->setCheckable(true); actFollowTrain->setChecked(false);
    tb->addAction(tr("Center"), this, [this]{ mnemonic->setOffset(0); });

    QSplitter *sp = new QSplitter(Qt::Vertical, this);
    mnemonic = new MnemonicWidget(this);
    sp->addWidget(mnemonic);

    ptLog = new QPlainTextEdit(this);
    ptLog->setReadOnly(true); ptLog->setMaximumBlockCount(2000); ptLog->setMaximumHeight(150);
    sp->addWidget(ptLog);
    setCentralWidget(sp);

    lblStatus = new QLabel(tr("Disconnected"));
    statusBar()->addPermanentWidget(lblStatus);

    tcp_client = new TcpClient(this);

    connect(tcp_client, &TcpClient::connected, this, &MainWindow::slotConnected);
    connect(tcp_client, &TcpClient::disconnected, this, &MainWindow::slotDisconnected);
    connect(tcp_client, &TcpClient::sendLogMessage, this, &MainWindow::slotRecvLogMessage);
    connect(tcp_client, &TcpClient::setStationsData, this, &MainWindow::slotStationsData);

    // Таймер для кoалесцирования обновлений мнемосхемы
    QTimer *updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(0);
    connect(updateTimer, &QTimer::timeout, mnemonic, [this]{ mnemonic->update(); });

    auto queueUpdate = [updateTimer]{
        if (!updateTimer->isActive())
            updateTimer->start();
    };

    connect(tcp_client, &TcpClient::setSignalsData, this, [this, queueUpdate](QByteArray &d)
    {
        dispatcher_data_t::deserializeSignals(d, m_disp_data);
        if (m_ready) mnemonic->setDispatcherData(&m_disp_data);
        queueUpdate();
    });

    connect(tcp_client, &TcpClient::setSwitchState, this, [this, queueUpdate](QByteArray &d)
    {
        if (!m_ready) return;
        switch_view_t sw;
        if (dispatcher_data_t::deserializeSwitch(d, sw))
            m_disp_data.switches[sw.name] = sw;
        queueUpdate();
    });

    connect(tcp_client, &TcpClient::setTrajBusyState, this, [this, queueUpdate](QByteArray &d)
    {
        if (!m_ready) return;
        traj_busy_view_t tb;
        if (dispatcher_data_t::deserializeTrajBusy(d, tb))
            m_disp_data.trajectory_states[tb.name] = tb;
        queueUpdate();
    });

    connect(tcp_client, &TcpClient::setTrainInfo, this, [this, queueUpdate](QByteArray &d)
    {
        if (!m_ready) return;
        dispatcher_data_t::deserializeTrains(d, m_disp_data.trains);
        queueUpdate();
    });

    connect(tcp_client, &TcpClient::setVehiclesPositions, this, [this, queueUpdate](QByteArray &d)
    {
        if (!m_ready) return;
        dispatcher_data_t::deserializePos(d, m_disp_data.pos_update);
        if (actFollowTrain->isChecked() && follow_vehicle >= 0 &&
            follow_vehicle < m_disp_data.pos_update.vehicles.size())
        {
            const auto &v = m_disp_data.pos_update.vehicles[follow_vehicle];
            if (!station_model.stations().empty())
            {
                const auto &ss = station_model.stations();
                int ni = 0; double md = 1e9;
                for (int i = 0; i < ss.size(); ++i)
                {
                    double dd = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
                    if (dd < md) { md = dd; ni = i; }
                }
                double to = -(ni * 300.0 * mnemonic->scale()) + (v.pos_x - ss[ni].pos_x) * 0.5 * mnemonic->scale();
                mnemonic->setOffset(to);
            }
        }
        mnemonic->update();
    });

    // Правый клик по сигналу — открыть/закрыть
    connect(mnemonic, &MnemonicWidget::signalRightClicked, this, [this](const signal_view_t *sig)
    {
        if (!sig) return;
        QMenu menu(this);
        QAction *actOpen = menu.addAction(tr("Open signal"));
        QAction *actClose = menu.addAction(tr("Close signal"));
        QAction *chosen = menu.exec(QCursor::pos());
        if (!chosen) return;

        QByteArray cmd;
        QDataStream st(&cmd, QIODevice::WriteOnly);
        st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
        st << sig->conn_name << sig->signal_dir
           << (chosen == actOpen) // open_train
           << false // open_shunting
           << false // open_call
           << (chosen == actClose); // close
        tcp_client->sendSignalCommand(cmd);
        ptLog->appendPlainText(tr("Signal %1: %2").arg(sig->letter).arg(chosen == actOpen ? tr("OPEN") : tr("CLOSE")));
    });

    // Правый клик по коннектору (стрелке) — перевести
    connect(mnemonic, &MnemonicWidget::connectorRightClicked, this, [this](const QString &name)
    {
        QMenu menu(this);
        QAction *actPlus = menu.addAction(tr("Set + (straight)"));
        QAction *actMinus = menu.addAction(tr("Set - (diverging)"));
        QAction *chosen = menu.exec(QCursor::pos());
        if (!chosen) return;

        int8_t dir = 1; // FWD
        int8_t state = (chosen == actPlus) ? 2 : -2; // STATE_PLUS or STATE_MINUS

        QByteArray cmd;
        QDataStream st(&cmd, QIODevice::WriteOnly);
        st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
        st << name << dir << state;
        tcp_client->sendSwitchCommand(cmd);
        ptLog->appendPlainText(tr("Switch %1: %2").arg(name).arg(chosen == actPlus ? tr("STRAIGHT") : tr("DIVERGING")));
    });

    // Правый клик по поезду — переименовать/развернуть
    connect(mnemonic, &MnemonicWidget::trainRightClicked, this, [this](int vehicle_id)
    {
        // Ищем поезд, содержащий эту ПЕ
        int train_idx = -1;
        for (int i = 0; i < m_disp_data.trains.size(); ++i)
        {
            const auto &t = m_disp_data.trains[i];
            if (vehicle_id >= t.first_vehicle && vehicle_id <= t.last_vehicle)
            { train_idx = i; break; }
        }
        if (train_idx < 0) return;

        QMenu menu(this);
        QAction *actFollow = menu.addAction(tr("Follow this train"));
        QAction *actRename = menu.addAction(tr("Rename"));
        QAction *actReverse = menu.addAction(tr("Reverse"));
        QAction *actCenter = menu.addAction(tr("Center on train"));
        QAction *chosen = menu.exec(QCursor::pos());
        if (!chosen) return;

        if (chosen == actFollow)
        {
            actFollowTrain->setChecked(true);
            follow_vehicle = vehicle_id;
            ptLog->appendPlainText(tr("Following train #%1").arg(train_idx));
        }
        else if (chosen == actCenter)
        {
            const auto &v = m_disp_data.pos_update.vehicles[vehicle_id];
            if (!station_model.stations().empty())
            {
                const auto &ss = station_model.stations();
                int ni = 0; double md = 1e9;
                for (int i = 0; i < ss.size(); ++i)
                {
                    double dd = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
                    if (dd < md) { md = dd; ni = i; }
                }
                double to = -(ni * 300.0 * mnemonic->scale()) + (v.pos_x - ss[ni].pos_x) * 0.5 * mnemonic->scale();
                mnemonic->setOffset(to);
            }
        }
        else if (chosen == actRename)
        {
            bool ok;
            QString newName = QInputDialog::getText(this, tr("Rename train"),
                                                     tr("New name:"), QLineEdit::Normal,
                                                     m_disp_data.trains[train_idx].name, &ok);
            if (ok && !newName.isEmpty())
                tcp_client->sendNewTrainName(train_idx, newName);
        }
        else if (chosen == actReverse)
        {
            tcp_client->sendReverseTrain(train_idx);
        }
    });

    // Клик по станции — центрировать
    connect(mnemonic, &MnemonicWidget::stationLeftClicked, this, [this](const QString &name)
    {
        for (int i = 0; i < station_model.stations().size(); ++i)
        {
            if (station_model.stations()[i].name == name)
            {
                double to = -i * 300.0 * mnemonic->scale() + (width() - 600) / 4.0;
                mnemonic->setOffset(to);
                ptLog->appendPlainText(tr("Centered on %1").arg(name));
                return;
            }
        }
    });

    connect(actFollowTrain, &QAction::toggled, this, [this](bool checked)
    {
        if (checked && !m_disp_data.pos_update.vehicles.empty())
            follow_vehicle = 0;
        else
            follow_vehicle = -1;
    });

    loadSettingsGUI();
    loadConfig("../cfg/dispatcher-console-tcp.xml");

    // Таймер переподключения
    QTimer *reconnectTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, [this]{
        if (tcp_client && !tcp_client->isConnected())
            tcp_client->slotConnect();
    });
    reconnectTimer->start(5000);

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
        QString tn;
        if (!cfg.getString("GUISettings", "Theme", tn)) tn = "dark-jedy";
        std::string tp = fs.combinePath(fs.getThemeDir(), tn.toStdString() + ".qss");
        QFile f(QString(tp.c_str()));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            setStyleSheet(f.readAll()); f.close();
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
    m_ready = false;
    ptLog->appendPlainText(tr("=== Connected ==="));
    updateStatus(tr("Connected"));
    tcp_client->sendRequest(STYPE_REQUEST_STATIONS_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_TOPOLOGY_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);
    tcp_client->sendRequest(STYPE_REQUEST_TRAINS_UPDATE);
    QTimer::singleShot(100, this, [this]{
        m_ready = true;
    });
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, 0.07);
}

void MainWindow::slotDisconnected()
{
    ptLog->appendPlainText(tr("=== Disconnected ==="));
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
    ptLog->appendPlainText(tr("Stations (%1):").arg(sv.size()));

    mStations->clear();
    for (int si = 0; si < sv.size(); ++si)
    {
        const auto &s = sv[si];
        ptLog->appendPlainText(tr("  %1").arg(s.name));
        QAction *a = mStations->addAction(s.name);
        int idx = si;
        connect(a, &QAction::triggered, this, [this, idx]{
            double to = -idx * 300.0 * mnemonic->scale();
            mnemonic->setOffset(to);
        });
    }
}

void MainWindow::slotRecvLogMessage(QString msg) { ptLog->appendPlainText(msg); }