//------------------------------------------------------------------------------
//
//      RSS launcher main window
//      (c) maisvendoo, 17/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief RSS launcher main window
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 17/12/2018
 */

#include    "mainwindow.h"
#include    "datetime.h"
#include    "train-waypoint-widget.h"
#include    "ui_mainwindow.h"

#include    <QPushButton>
#include    <QDir>
#include    <QDirIterator>
#include    <QStringList>
#include    <QTextStream>

#include    "filesystem.h"
#include    "CfgReader.h"

#include    "platform.h"
#include    "styles.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tbActiveTrains = new QToolBox(this);
    int idx_before_last = ui->vblActiveTrainsLayout->count() - 1;
    ui->vblActiveTrainsLayout->insertWidget(idx_before_last, tbActiveTrains);

    init();

    connect(ui->lwRoutes, &QListWidget::itemSelectionChanged,
            this, &MainWindow::slotRouteSelection);

    connect(ui->lwTrains, &QListWidget::itemSelectionChanged,
            this, &MainWindow::slotTrainSelection);

    connect(ui->cbStartDateManually, &QCheckBox::stateChanged,
            this, &MainWindow::slotStartDateManuallyChanged);

    connect(ui->cbStartTimeManually, &QCheckBox::stateChanged,
            this, &MainWindow::slotStartTimeManuallyChanged);

    connect(ui->pbStartServer, &QPushButton::pressed,
            this, &MainWindow::slotStartServerPressed);

    connect(ui->pbStartViewer, &QPushButton::pressed,
            this, &MainWindow::slotStartViewerPressed);

    connect(ui->pbStartMap, &QPushButton::pressed,
            this, &MainWindow::slotStartMapPressed);

    connect(&simulatorProc, &QProcess::started,
            this, &MainWindow::slotSimulatorStarted);

    connect(&viewerProc, &QProcess::started,
            this, &MainWindow::slotViewerStarted);

    connect(&mapProc, &QProcess::started,
            this, &MainWindow::slotMapStarted);

    connect(&simulatorProc, &QProcess::finished,
            this, &MainWindow::slotSimulatorFinished);

    connect(&viewerProc, &QProcess::finished,
            this, &MainWindow::slotViewerFinished);

    connect(&mapProc, &QProcess::finished,
            this, &MainWindow::slotMapFinished);

    connect(ui->pbConnectViewer, &QPushButton::pressed,
            this, &MainWindow::slotConnectViewerPressed);

    connect(ui->pbConnectMap, &QPushButton::pressed,
            this, &MainWindow::slotConnectMapPressed);

    connect(ui->cbSavedServers, &QComboBox::currentIndexChanged,
            this, &MainWindow::slotSelectSavedServer);

    connect(ui->leServerName, &QLineEdit::textChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->sbIPv4_1, &QSpinBox::valueChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->sbIPv4_2, &QSpinBox::valueChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->sbIPv4_3, &QSpinBox::valueChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->sbIPv4_4, &QSpinBox::valueChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->sbIPv4_port, &QSpinBox::valueChanged,
            this, &MainWindow::slotChangedServerSettings);

    connect(ui->pbSaveServer, &QPushButton::pressed,
            this, &MainWindow::slotSaveServer);

    connect(ui->spWidth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spHeight, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbFullScreen, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbDoubleBuffer, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbVSync, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbWindowDecoration, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->dspFovY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, QOverload<double>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->dspNear, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, QOverload<double>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spViewDist, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spScreenNumber, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->pbCancel, &QPushButton::released, this, &MainWindow::slotCancelGraphSettings);
    connect(ui->pbApply, &QPushButton::released, this, &MainWindow::slotApplyGraphSettings);

    connect(ui->pbAddTrain, &QPushButton::released, this, &MainWindow::slotAddActiveTrain);
    connect(ui->pbDeleteTrain, &QPushButton::released, this, &MainWindow::slotDeleteActiveTrain);

    setCentralWidget(ui->twMain);

    setFocusPolicy(Qt::ClickFocus);

    loadTheme();

    QIcon icon(":/images/images/RRS_logo.png");
    setWindowIcon(icon);

    QTimeZone local_zone = QTimeZone::systemTimeZone();
    QDateTime current = QDateTime::currentDateTime(local_zone);
    ui->dteStartDate->setDateTime(current);
    ui->dteStartDate->setTimeZone(local_zone);
    ui->dteStartTime->setDateTime(current);
    ui->dteStartTime->setTimeZone(local_zone);
    connect(&update_datetime_timer, &QTimer::timeout, this, &MainWindow::slotUpdateDateTime);
    update_datetime_timer.start(300);

    ui->dteStartDate->setEnabled(false);
    ui->dteStartTime->setEnabled(false);
    ui->pbAddTrain->setEnabled(false);
    ui->pbDeleteTrain->setEnabled(false);
    ui->pbStartViewer->setEnabled(false);
    ui->pbStartMap->setEnabled(false);
    ui->pbStartServer->setEnabled(false);
    is_start_button_to_stop_server = false;

    connect(ui->cbScenario, &QComboBox::currentIndexChanged, this, &MainWindow::slotOnScenarioSelection);

    ui->tbScenarioDescription->setTextInteractionFlags(Qt::NoTextInteraction);
    ui->tbScenarioDescription->setFocusPolicy(Qt::NoFocus);
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
void MainWindow::init()
{
    FileSystem &fs = FileSystem::getInstance();

    loadRoutesList(fs.getRouteRootDir());
    loadTrainsList(fs.getTrainsDir());
    loadServersList(fs.getConfigDir());

    loadGraphicsSettings("settings");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadRoutesList(const std::string &routesDir)
{
    QDir routes(QString(routesDir.c_str()));
    QDirIterator route_dirs(routes.path(), QStringList(), QDir::NoDotAndDotDot | QDir::Dirs);

    while (route_dirs.hasNext())
    {
        route_info_t route_info;
        route_info.route_dir_full_path = route_dirs.next();
        route_info.route_dir_name = route_dirs.fileName();

        CfgReader cfg;

        if (cfg.load(route_info.route_dir_full_path + QDir::separator() + "description.xml"))
        {
            QString secName = "Route";

            cfg.getString(secName, "Title", route_info.route_title);
            cfg.getString(secName, "Description", route_info.route_description);
        }

        loadTrajectories(route_info);
        loadTrainPositions(route_info);
        loadScenarios(route_info);

        routes_info.push_back(route_info);
    }

    for (auto it = routes_info.begin(); it != routes_info.end(); ++it)
    {
        ui->lwRoutes->addItem((*it).route_title);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadTrainsList(const std::string &trainsDir)
{
    QDir    trains(QString(trainsDir.c_str()));
    QDirIterator train_files(trains.path(), QStringList() << "*.xml", QDir::NoDotAndDotDot | QDir::Files);

    while (train_files.hasNext())
    {
        train_info_t train_info;
        QString fullPath = train_files.next();
        QFileInfo fileInfo(fullPath);

        train_info.train_config_path = fileInfo.baseName();

        CfgReader cfg;

        if (cfg.load(fullPath))
        {
            QString secName = "Common";

            cfg.getString(secName, "Title", train_info.train_title);
            cfg.getString(secName, "Description", train_info.description);
        }

        trains_info.push_back(train_info);
        ui->lwTrains->addItem(train_info.train_title);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadServersList(const std::string &cfgDir)
{
    saved_servers.clear();
    ui->cbSavedServers->clear();

    saved_servers_path = QString(cfgDir.c_str()) + QDir::separator() + QString(SAVED_SERVERS_FILE.c_str());
    CfgReader cfg;
    if (cfg.load(saved_servers_path))
    {
        QDomNode server_node = cfg.getFirstSection("Server");
        while (!server_node.isNull())
        {
            server_info_t server = server_info_t();

            QString tmp_host_address = "";
            int tmp_port = 0;

            cfg.getString(server_node, "Name", server.server_name);

            if (cfg.getString(server_node, "HostAddr", tmp_host_address))
                server.setHostAddress(tmp_host_address);

            if (cfg.getInt(server_node, "port", tmp_port))
                server.ipv4_port = static_cast<uint16_t>(tmp_port);

            saved_servers.insert(server.server_name, server);

            server_node = cfg.getNextSection();
        }
    }

    if (saved_servers.empty())
    {
        server_info_t local_server = server_info_t();
        saved_servers.insert(local_server.server_name, local_server);
    }

    for (auto& ss : saved_servers)
    {
        ui->cbSavedServers->addItem(ss.server_name + " (" + ss.getHostAddressAndPort() + ")");
    }

    slotSelectSavedServer(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadTrajectories(route_info_t &route_info)
{
    route_info.trajectrories.clear();

    QString traj_dir_path = route_info.route_dir_full_path + QDir::separator() +
                            "topology" + QDir::separator() +
                            "trajectories";

    QDir traj_dir(traj_dir_path);
    QDirIterator traj_files(traj_dir.path(), QStringList() << "*.traj", QDir::NoDotAndDotDot | QDir::Files);
    while (traj_files.hasNext())
    {
        trajectory_info_t traj_info;
        QString fullPath = traj_files.next();
        QFileInfo fileInfo(fullPath);
        traj_info.name = fileInfo.baseName();
        route_info.trajectrories.push_back(traj_info);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadTrainPositions(route_info_t &route_info)
{
    route_info.fwd_train_positions.clear();
    route_info.bwd_train_positions.clear();

    QString path = route_info.route_dir_full_path + QDir::separator() +
                   "topology" + QDir::separator() +
                   "waypoints.conf";

    QFile waypoints_file(path);

    if (!waypoints_file.open(QIODevice::ReadOnly))
    {
        return;
    }

    QTextStream stream(&waypoints_file);

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QStringList tokens = line.split('\t');

        train_position_t tp;
        tp.name = tokens[0];
        tp.trajectory_name = tokens[1];
        tp.direction = tokens[2].toInt();
        tp.traj_coord = tokens[3].toDouble();
        tp.railway_coord = tokens[4].toDouble();

        if (tp.direction > 0)
        {
            route_info.fwd_train_positions.push_back(tp);
        }
        else
        {
            route_info.bwd_train_positions.push_back(tp);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadScenarios(route_info_t &route_info)
{
    route_info.scenarios.clear();

    QString scenarios_path = route_info.route_dir_full_path + QDir::separator()
                             + "scenarios";

    scenarios_path = QDir::toNativeSeparators(scenarios_path);

    QDir scenarios_dir(scenarios_path);

    if (!scenarios_dir.exists())
    {
        return;
    }

    // Перебираем все подкаталоги в папке scenarios данного маршрута
    QDirIterator it(scenarios_path, QDir::Dirs | QDir::NoDotAndDotDot);

    while (it.hasNext())
    {
        scenario_t scn;

        // Проверяем наличие в найденном подкаталоге файла main.lua
        QString abs_path = it.next();
        QString main_path = QDir::toNativeSeparators(abs_path + QDir::separator() + "main.lua");
        QFile main_file(main_path);

        if (main_file.exists())
        {
            // И только в случае наличия такового - добавляем каталог в список
            // доступных сценариев
            scn.scenario_name = scenarios_dir.relativeFilePath(abs_path);

            // Читаем описание сценария из README.md
            QString desc_path = QDir::toNativeSeparators(abs_path + QDir::separator() + "README.md");
            scn.scenario_description = loadScenarioDescription(desc_path);

            route_info.scenarios.push_back(scn);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::loadScenarioDescription(QString path)
{
    QString desc = "";

    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        desc = QString::fromUtf8(file.readAll());
    }

    return desc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::clearActiveTrainsList()
{
    if (!is_start_button_to_stop_server)
        ui->pbStartServer->setEnabled(false);

    ui->pbDeleteTrain->setEnabled(false);

    active_trains.clear();

    while (tbActiveTrains->count() > 0)
    {
        TrainWaypointWidget *tww = dynamic_cast<TrainWaypointWidget *>(tbActiveTrains->widget(0));
        disconnect(tww, &TrainWaypointWidget::activeTrainChanged,
                   this, &MainWindow::slotUpdateActiveTrains);
        disconnect(tww, &TrainWaypointWidget::trainConfigChanged,
                   this, &MainWindow::slotTrainConfigChanged);

        tbActiveTrains->removeItem(0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadActiveTrainsList()
{
    if ((selected_route_idx < 0) || (selected_route_idx >= routes_info.size()))
    {
        return;
    }

    ui->cbScenario->clear();
    ui->cbScenario->addItem("<Not_selected>");

    for (auto& sc : routes_info[selected_route_idx].scenarios)
    {
        ui->cbScenario->addItem(sc.scenario_name);        
    }

    slotUpdateActiveTrains();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startSimulator()
{
    if (selectedRouteDirName.isEmpty())
    {
        return;
    }

    bool reset_start_config = false;

    slotUpdateActiveTrains(reset_start_config);

    if (active_trains.empty() && selected_scenario_idx < 0)
    {
        return;
    }

    QStringList args;
    args.clear();

    args << "--route=" + selectedRouteDirName;

    if (selected_scenario_idx >= 0)
    {
        args << "--scenario=" + routes_info[selected_route_idx].scenarios[selected_scenario_idx].scenario_name;
    }
    else
    {
        server_date_t start_date = server_date_t(
            static_cast<int16_t>(ui->dteStartDate->dateTime().date().year()),
            static_cast<uint8_t>(ui->dteStartDate->dateTime().date().month()),
            static_cast<uint8_t>(ui->dteStartDate->dateTime().date().day()));
        server_time_t start_time = server_time_t(
            static_cast<uint8_t>(ui->dteStartTime->dateTime().time().hour()),
            static_cast<uint8_t>(ui->dteStartTime->dateTime().time().minute()),
            static_cast<uint8_t>(ui->dteStartTime->dateTime().time().second()),
            static_cast<uint16_t>(ui->dteStartTime->dateTime().time().msec()));
        std::int64_t start_datetime = simulator_time_t(start_date, start_time).data();

        args << "--start=" + QString::number(start_datetime);

        createTmpScenario(selectedRouteDirName, active_trains);

        args << "--scenario=" + STARTUP_SCN_SUBDIR;
    }

    FileSystem &fs = FileSystem::getInstance();
    QString simPath = SIMULATOR_NAME + EXE_EXP;

    simulatorProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    simulatorProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + simPath, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startViewer(bool local)
{
    server_info_t server;
    if (local)
    {
        server = server_info_t();
    }
    else
    {
        server.ipv4_1 = ui->sbIPv4_1->value();
        server.ipv4_2 = ui->sbIPv4_2->value();
        server.ipv4_3 = ui->sbIPv4_3->value();
        server.ipv4_4 = ui->sbIPv4_4->value();
        server.ipv4_port = ui->sbIPv4_port->value();
    }

    QStringList args;
    args << "--host-address" << server.getHostAddress();
    args << "--port" << QString::number(server.ipv4_port);

    FileSystem &fs = FileSystem::getInstance();
    QString viewerPath = VIEWER_NAME + EXE_EXP;

    if (local)
    {
        viewerProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
        viewerProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + viewerPath, args);
    }
    else
    {
        QProcess *proc = new QProcess;
        connect(proc, &QProcess::finished, this, &MainWindow::slotAdditionalProcFinished);
        proc->setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
        proc->start(QString::fromStdString(fs.getBinaryDir()) + '/' + viewerPath, args);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startMap(bool local)
{
    server_info_t server;
    if (local)
    {
        server = server_info_t();
    }
    else
    {
        server.ipv4_1 = ui->sbIPv4_1->value();
        server.ipv4_2 = ui->sbIPv4_2->value();
        server.ipv4_3 = ui->sbIPv4_3->value();
        server.ipv4_4 = ui->sbIPv4_4->value();
        server.ipv4_port = ui->sbIPv4_port->value();
    }

    QStringList args;
    args << "--host-address" << server.getHostAddress();
    args << "--port" << QString::number(server.ipv4_port);

    FileSystem &fs = FileSystem::getInstance();
    QString mapPath = ROUTE_MAP_NAME + EXE_EXP;

    if (local)
    {
        mapProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
        mapProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + mapPath, args);
    }
    else
    {
        QProcess *proc = new QProcess;
        connect(proc, &QProcess::finished, this, &MainWindow::slotAdditionalProcFinished);
        proc->setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
        proc->start(QString::fromStdString(fs.getBinaryDir()) + '/' + mapPath, args);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadTheme()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_dir = fs.getConfigDir();
    std::string cfg_path = fs.combinePath(cfg_dir, "launcher.xml");

    CfgReader cfg;

    if ( cfg.load(QString(cfg_path.c_str())) )
    {
        QString secName = "Launcher";
        QString theme_name = "";

        if (!cfg.getString(secName, "Theme", theme_name))
        {
            theme_name = "dark-jedy";
        }

        std::string theme_dir = fs.getThemeDir();
        std::string theme_path = fs.combinePath(theme_dir, theme_name.toStdString() + ".qss");
        QString style_sheet = readStyleSheet(QString(theme_path.c_str()));

        this->setStyleSheet(style_sheet);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRouteSelection()
{
    int route_idx = ui->lwRoutes->currentRow();
    if (selected_route_idx == route_idx)
        return;

    // Очистка выбранных активных поездов
    clearActiveTrainsList();
    ui->ptRouteDescription->clear();

    if (route_idx == -1)
    {
        selected_route_idx = -1;
        return;
    }

    selectedRouteDirName = routes_info[route_idx].route_dir_name;
    ui->ptRouteDescription->appendPlainText(routes_info[route_idx].route_description);

    trajectrories = &routes_info[route_idx].trajectrories;
    fwd_train_positions = &routes_info[route_idx].fwd_train_positions;
    bwd_train_positions = &routes_info[route_idx].bwd_train_positions;

    // Загрузка предыдущих выбранных активных поездов
    selected_route_idx = route_idx;
    loadActiveTrainsList();

    ui->pbAddTrain->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTrainSelection()
{
    size_t item_idx = static_cast<size_t>(ui->lwTrains->currentRow());

    ui->ptTrainDescription->clear();
    selectedTrain = trains_info[item_idx].train_config_path;
    ui->ptTrainDescription->appendPlainText(trains_info[item_idx].description);

    ui->pbAddTrain->setEnabled(ui->lwRoutes->currentRow() >= 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAddActiveTrain()
{
    if (ui->lwRoutes->currentRow() < 0)
        return;

    TrainWaypointWidget *tww = new TrainWaypointWidget(&trains_info,
                                                       trajectrories,
                                                       fwd_train_positions,
                                                       bwd_train_positions,
                                                       &icon_ok,
                                                       &icon_cancel,
                                                       &icon_warn,
                                                       this);

    int train_idx = ui->lwTrains->currentRow();
    if ((tww->cbTrainConfigSelect->count() > train_idx) && (train_idx >= 0))
        tww->cbTrainConfigSelect->setCurrentIndex(train_idx + 1);

    int new_item_idx = tbActiveTrains->addItem(tww, tww->getTrainName());
    tbActiveTrains->setCurrentIndex(new_item_idx);
    connect(tww, &TrainWaypointWidget::activeTrainChanged,
               this, &MainWindow::slotUpdateActiveTrains);
    connect(tww, &TrainWaypointWidget::trainConfigChanged,
            this, &MainWindow::slotTrainConfigChanged);

    slotUpdateActiveTrains();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDeleteActiveTrain()
{
    if (tbActiveTrains->count() <= 0)
        return;

    int cur = tbActiveTrains->currentIndex();
    TrainWaypointWidget *tww = dynamic_cast<TrainWaypointWidget *>(tbActiveTrains->widget(cur));
    if (tww)
    {
        tbActiveTrains->removeItem(cur);
        disconnect(tww, &TrainWaypointWidget::activeTrainChanged,
                   this, &MainWindow::slotUpdateActiveTrains);
        disconnect(tww, &TrainWaypointWidget::trainConfigChanged,
                   this, &MainWindow::slotTrainConfigChanged);
        delete tww;
    }

    slotUpdateActiveTrains();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSelectSavedStartConfig(int idx)
{
    if (idx == new_added_start_config_idx)
    {
        new_added_start_config_idx = -1;
        return;
    }

    if (idx > 0)
    {
        clearActiveTrainsList();        
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTrainConfigChanged()
{
    TrainWaypointWidget *tww = dynamic_cast<TrainWaypointWidget *>(sender());
    if (tww)
    {
        int idx = tbActiveTrains->indexOf(tww);
        if (idx == -1)
            return;

        tbActiveTrains->setItemText(idx, tww->getTrainName());

        int train_idx = tww->cbTrainConfigSelect->currentIndex() - 1;
        if ((train_idx >= 0) && (train_idx < trains_info.size()))
        {
            ui->lwTrains->setCurrentRow(train_idx, QItemSelectionModel::ClearAndSelect);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotUpdateActiveTrains(bool reset_start_config)
{
    active_trains.clear();

    int active_trains_count = tbActiveTrains->count();
    if (active_trains_count <= 0)
    {
        if (!is_start_button_to_stop_server)
            ui->pbStartServer->setEnabled(false);

        ui->pbDeleteTrain->setEnabled(false);

        return;
    }

    ui->pbDeleteTrain->setEnabled(true);

    for (int i = 0; i < active_trains_count; ++i)
    {
        TrainWaypointWidget *tww = dynamic_cast<TrainWaypointWidget *>(tbActiveTrains->widget(i));
        if (tww)
        {
            active_train_t at = tww->getActiveTrainConfig();
            if (at.is_active)
            {
                active_trains.push_back(at);
                tbActiveTrains->setItemIcon(i, icon_ok);
            }
            else
            {
                tbActiveTrains->setItemIcon(i, icon_warn);
            }
        }
    }

    if (!is_start_button_to_stop_server)
        ui->pbStartServer->setEnabled(!active_trains.empty() || selected_scenario_idx >= 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotStartDateManuallyChanged()
{
    ui->dteStartDate->setEnabled(ui->cbStartDateManually->isChecked());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotStartTimeManuallyChanged()
{
    ui->dteStartTime->setEnabled(ui->cbStartTimeManually->isChecked());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotUpdateDateTime()
{
    QDateTime current = QDateTime::currentDateTime(QTimeZone::systemTimeZone());

    if (!ui->cbStartDateManually->isChecked())
        ui->dteStartDate->setDateTime(current);

    if (!ui->cbStartTimeManually->isChecked())
        ui->dteStartTime->setDateTime(current);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotStartServerPressed()
{
    // Check button is stop running server mode
    if (is_start_button_to_stop_server)
    {
        simulatorProc.kill();
        return;
    }

    // Check is route selected
    if (selectedRouteDirName.isEmpty())
    {
        return;
    }

    startSimulator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotStartViewerPressed()
{
    startViewer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotStartMapPressed()
{
    startMap();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSimulatorStarted()
{
    is_start_button_to_stop_server = true;
    ui->pbStartServer->setStyleSheet("background-color: red;");
    ui->pbStartServer->setText(tr("Stop server"));
    ui->pbStartServer->setEnabled(true);

    if (ui->cbAutostartViewer->isChecked())
        startViewer();
    else
        ui->pbStartViewer->setEnabled(true);

    if (ui->cbAutostartMap->isChecked())
        startMap();
    else
        ui->pbStartMap->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotViewerStarted()
{
    ui->pbStartViewer->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotMapStarted()
{
    ui->pbStartMap->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnectViewerPressed()
{
    bool local = false;
    startViewer(local);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnectMapPressed()
{
    bool local = false;
    startMap(local);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    is_start_button_to_stop_server = false;
    ui->pbStartServer->setStyleSheet("background-color: ;");
    ui->pbStartServer->setText(tr("Start server"));

    if (active_trains.empty() || (ui->lwRoutes->currentRow() < 0))
    {
        ui->pbStartServer->setEnabled(false);
    }
    else
    {
        ui->pbStartServer->setEnabled(true);
    }

    if (selected_scenario_idx >= 0)
    {
        ui->pbStartServer->setEnabled(true);
    }

    ui->pbStartViewer->setEnabled(false);
    ui->pbStartMap->setEnabled(false);

    if (viewerProc.state() != QProcess::NotRunning)
        viewerProc.kill();

    if (mapProc.state() != QProcess::NotRunning)
        mapProc.kill();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotViewerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    if (simulatorProc.state() != QProcess::NotRunning)
        ui->pbStartViewer->setEnabled(true);

    //simulatorProc.kill();
    setFocusPolicy(Qt::StrongFocus);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotMapFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    if (simulatorProc.state() != QProcess::NotRunning)
        ui->pbStartMap->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAdditionalProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *proc = dynamic_cast<QProcess *>(sender());
    disconnect(proc, &QProcess::finished, this, &MainWindow::slotAdditionalProcFinished);
    delete proc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSelectSavedServer(int idx)
{
    server_info_t server = server_info_t();
    int i = 0;
    for (auto& ss : saved_servers)
    {
        if (i == idx)
        {
            server = ss;
            break;
        }
        ++i;
    }
    ui->cbSavedServers->setCurrentIndex(idx);
    ui->leServerName->setText(server.server_name);
    ui->sbIPv4_1->setValue(server.ipv4_1);
    ui->sbIPv4_2->setValue(server.ipv4_2);
    ui->sbIPv4_3->setValue(server.ipv4_3);
    ui->sbIPv4_4->setValue(server.ipv4_4);
    ui->sbIPv4_port->setValue(server.ipv4_port);
    slotChangedServerSettings();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotChangedServerSettings()
{
    if (ui->leServerName->text().isEmpty())
    {
        ui->pbSaveServer->setText(tr("Save Server"));
        ui->pbSaveServer->setEnabled(false);
    }
    else
    {
        if (saved_servers.contains(ui->leServerName->text()))
        {
            ui->pbSaveServer->setText(tr("Rewrite Server"));

            server_info_t saved_server = saved_servers.value(ui->leServerName->text());
            if ((saved_server.ipv4_1 == ui->sbIPv4_1->value()) &&
                (saved_server.ipv4_2 == ui->sbIPv4_2->value()) &&
                (saved_server.ipv4_3 == ui->sbIPv4_3->value()) &&
                (saved_server.ipv4_4 == ui->sbIPv4_4->value()) &&
                (saved_server.ipv4_port == ui->sbIPv4_port->value()))
            {
                ui->pbSaveServer->setEnabled(false);
            }
            else
            {
                ui->pbSaveServer->setEnabled(true);
            }
        }
        else
        {
            ui->pbSaveServer->setEnabled(true);
            ui->pbSaveServer->setText(tr("Save Server"));
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveServer()
{
    if (ui->leServerName->text().isEmpty())
        return;

    server_info_t server = server_info_t();
    server.server_name = ui->leServerName->text();
    server.ipv4_1 = ui->sbIPv4_1->value();
    server.ipv4_2 = ui->sbIPv4_2->value();
    server.ipv4_3 = ui->sbIPv4_3->value();
    server.ipv4_4 = ui->sbIPv4_4->value();
    server.ipv4_port = ui->sbIPv4_port->value();

    auto saved_it = saved_servers.insert(server.server_name, server);
    int idx = ui->cbSavedServers->currentIndex();

    CfgEditor editor;
    editor.openFileForWrite(saved_servers_path);
    editor.setIndentationFormat(-1);

    ui->cbSavedServers->clear();
    int i = 0;
    for (auto it = saved_servers.begin(); it != saved_servers.end(); ++it)
    {
        if (it == saved_it)
            idx = i;
        ++i;

        server_info_t server = it.value();
        ui->cbSavedServers->addItem(server.server_name + " (" + server.getHostAddressAndPort() + ")");

        FieldsDataList flist;
        flist.append(QPair<QString, QString>("Name", server.server_name));
        flist.append(QPair<QString, QString>("HostAddr", server.getHostAddress()));
        flist.append(QPair<QString, QString>("port", QString::number(server.ipv4_port)));
        editor.writeFile("Server", flist);
    }

    editor.closeFileAfterWrite();

    slotSelectSavedServer(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotChangedGraphSetting(int)
{
    ui->pbApply->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotChangedGraphSetting(double)
{
    ui->pbApply->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCancelGraphSettings()
{
    updateGraphSettings(fd_list, ui);
    ui->pbApply->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotApplyGraphSettings()
{
    applyGraphSettings(fd_list, ui);

    updateGraphSettings(fd_list, ui);

    saveGraphSettings(fd_list);

    ui->pbApply->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnScenarioSelection(int cur_idx)
{
    ui->tbScenarioDescription->clear();
    selected_scenario_idx = -1;

    if (cur_idx <= 0)
    {
        ui->pbStartServer->setEnabled(false);
        return;
    }

    QString desc = routes_info[selected_route_idx].scenarios[cur_idx - 1].scenario_description;
    ui->tbScenarioDescription->setMarkdown(desc);

    selected_scenario_idx = cur_idx - 1;

    ui->pbStartServer->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const   QString MainWindow::WIDTH = "Width";
const   QString MainWindow::HEIGHT = "Height";
const   QString MainWindow::FULLSCREEN = "FullScreen";
const   QString MainWindow::FOV_Y = "FovY";
const   QString MainWindow::ZNEAR = "zNear";
const   QString MainWindow::ZFAR = "zFar";
const   QString MainWindow::SCREEN_NUM = "ScreenNumber";
const   QString MainWindow::WIN_DECOR = "WindowDecoration";
const   QString MainWindow::DOUBLE_BUFF = "DoubleBuffer";
const   QString MainWindow::VSYNC = "VSync";
const   QString MainWindow::NOTIFY_LEVEL = "NofifyLevel";
const   QString MainWindow::VIEW_DIST = "ViewDistance";
const   QString MainWindow::STARTUP_SCN_SUBDIR = "startup";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadGraphicsSettings(QString file_name)
{
    FileSystem &fs = FileSystem::getInstance();
    QString config_dir = QString(fs.getConfigDir().c_str());

    settings_path = config_dir + fs.separator() + file_name + ".xml";

    QString secName = "Viewer";

    CfgReader   cfg;

    if (cfg.load(settings_path))
    {
        int width = 0;
        cfg.getInt(secName, WIDTH, width);
        fd_list.append(QPair<QString, QVariant>(WIDTH, width));

        int height = 0;
        cfg.getInt(secName, HEIGHT, height);
        fd_list.append(QPair<QString, QVariant>(HEIGHT, height));

        int fullscreen = 0;
        cfg.getInt(secName, FULLSCREEN, fullscreen);
        fd_list.append(QPair<QString, QVariant>(FULLSCREEN, fullscreen));

        double fovY = 0;
        cfg.getDouble(secName, FOV_Y, fovY);
        fd_list.append(QPair<QString, QVariant>(FOV_Y, fovY));

        double zNear = 0;
        cfg.getDouble(secName, ZNEAR, zNear);
        fd_list.append(QPair<QString, QVariant>(ZNEAR, zNear));

        double zFar = 0;
        cfg.getDouble(secName, ZFAR, zFar);
        fd_list.append(QPair<QString, QVariant>(ZFAR, zFar));

        int screen_num = 0;
        cfg.getInt(secName, SCREEN_NUM, screen_num);
        fd_list.append(QPair<QString, QVariant>(SCREEN_NUM, screen_num));

        int win_decor = 0;
        cfg.getInt(secName, WIN_DECOR, win_decor);
        fd_list.append(QPair<QString, QVariant>(WIN_DECOR, win_decor));

        int double_buff = 0;
        cfg.getInt(secName, DOUBLE_BUFF, double_buff);
        fd_list.append(QPair<QString, QVariant>(DOUBLE_BUFF, double_buff));

        int vsync = 0;
        cfg.getInt(secName, VSYNC, vsync);
        fd_list.append(QPair<QString, QVariant>(VSYNC, vsync));

        double view_dist = 0;
        cfg.getDouble(secName, VIEW_DIST, view_dist);
        fd_list.append(QPair<QString, QVariant>(VIEW_DIST, view_dist));

        updateGraphSettings(fd_list, ui);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPair<QString, QVariant> findSetting(QString setting,
                                     FieldsDataList &fd_list,
                                     int &idx)
{
    QPair<QString, QVariant> pair;

    for (int i = 0; i < fd_list.size(); ++i)
    {
        pair = fd_list[i];

        if (pair.first == setting)
        {
            idx = i;
            return pair;
        }
    }

    return pair;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPair<QString, QVariant> findSetting(QString setting, FieldsDataList &fd_list)
{
    int idx = 0;
    QPair<QString, QVariant> pair = findSetting(setting, fd_list, idx);

    return pair;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateGraphSettings(FieldsDataList &fd_list, Ui::MainWindow *ui)
{
    ui->spWidth->setValue(findSetting(WIDTH, fd_list).second.toInt());
    ui->spHeight->setValue(findSetting(HEIGHT, fd_list).second.toInt());

    findSetting(FULLSCREEN, fd_list).second == 1 ?
                ui->cbFullScreen->setCheckState(Qt::CheckState::Checked) :
                ui->cbFullScreen->setCheckState(Qt::CheckState::Unchecked);

    findSetting(WIN_DECOR, fd_list).second == 1 ?
                ui->cbWindowDecoration->setCheckState(Qt::CheckState::Checked) :
                ui->cbWindowDecoration->setCheckState(Qt::CheckState::Unchecked);

    findSetting(DOUBLE_BUFF, fd_list).second == 1 ?
                ui->cbDoubleBuffer->setCheckState(Qt::CheckState::Checked) :
                ui->cbDoubleBuffer->setCheckState(Qt::CheckState::Unchecked);

    findSetting(VSYNC, fd_list).second == 1 ?
        ui->cbVSync->setCheckState(Qt::CheckState::Checked) :
        ui->cbVSync->setCheckState(Qt::CheckState::Unchecked);

    ui->spScreenNumber->setValue(findSetting(SCREEN_NUM, fd_list).second.toInt());
    ui->dspFovY->setValue(findSetting(FOV_Y, fd_list).second.toDouble());
    ui->dspNear->setValue(findSetting(ZNEAR, fd_list).second.toDouble());

    ui->spViewDist->setValue(findSetting(VIEW_DIST, fd_list).second.toInt());

    ui->pbApply->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applyGraphSettings(FieldsDataList &fd_list, Ui::MainWindow *ui)
{
    int idx = 0;

    findSetting(WIDTH, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(WIDTH, ui->spWidth->value());

    findSetting(HEIGHT, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(HEIGHT, ui->spHeight->value());

    findSetting(FULLSCREEN, fd_list, idx);
    if (ui->cbFullScreen->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(FULLSCREEN, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(FULLSCREEN, 0);
    }

    findSetting(DOUBLE_BUFF, fd_list, idx);
    if (ui->cbDoubleBuffer->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(DOUBLE_BUFF, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(DOUBLE_BUFF, 0);
    }

    findSetting(VSYNC, fd_list, idx);
    if (ui->cbVSync->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(VSYNC, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(VSYNC, 0);
    }

    findSetting(WIN_DECOR, fd_list, idx);
    if (ui->cbWindowDecoration->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(WIN_DECOR, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(WIN_DECOR, 0);
    }

    findSetting(SCREEN_NUM, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(SCREEN_NUM, ui->spScreenNumber->value());

    findSetting(FOV_Y, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(FOV_Y, ui->dspFovY->value());

    findSetting(ZNEAR, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(ZNEAR, ui->dspNear->value());

    findSetting(VIEW_DIST, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(VIEW_DIST, ui->spViewDist->value());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::saveGraphSettings(FieldsDataList &fd_list)
{
    CfgEditor editor;

    editor.editFile(settings_path, "Viewer", fd_list);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::createLuaSetDate(QDateEdit *dateEdit)
{
    QString setDate = "";

    setDate = QString("setDate(\"%1.%2.%3\")")
                  .arg(dateEdit->dateTime().date().day(), 2, u'0')
                  .arg(dateEdit->dateTime().date().month(), 2, u'0')
                  .arg(dateEdit->dateTime().date().year(), 4, u'0');

    return setDate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::createLuaSetTime(QTimeEdit *timeEdit)
{
    QString setTime = "";

    setTime = QString("setTime(\"%1:%2:%3\")")
                  .arg(timeEdit->dateTime().time().hour(), 2, u'0')
                  .arg(timeEdit->dateTime().time().minute(), 2, u'0')
                  .arg(timeEdit->dateTime().time().second(), 2, u'0');

    return setTime;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList MainWindow::createLuaSetTrain(size_t idx, const active_train_t &at)
{
    QStringList setTrainCode;
    setTrainCode.clear();

    QString varName = QString("train%1").arg(idx);

    setTrainCode.append(varName + QString(" = TrainData.new()"));
    setTrainCode.append(varName + QString(".name = \"%1\"").arg(idx));
    setTrainCode.append(varName + QString(".config = \"%1\"").arg(at.train_info.train_config_path));
    setTrainCode.append(varName + QString(".traj = \"%1\"").arg(at.train_position.trajectory_name));
    setTrainCode.append(varName + QString(".coord = %1").arg(at.train_position.traj_coord));
    setTrainCode.append(varName + QString(".dir = %1").arg(at.train_position.direction));
    setTrainCode.append(QString("setTrain(%1)").arg(varName));

    return setTrainCode;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList MainWindow::createTmpScenarioCode(const std::vector<active_train_t> &active_trains)
{
    QStringList scnCode;
    scnCode.clear();

    for (size_t i = 0; i < active_trains.size(); ++i)
    {
        scnCode += createLuaSetTrain(i, active_trains[i]);
        scnCode.append("\n");
    }

    return scnCode;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::createTmpScenario(const QString &route_name,
                                   const std::vector<active_train_t> &active_trains)
{
    auto scnCode = createTmpScenarioCode(active_trains);

    if (scnCode.empty())
    {
        return;
    }

    FileSystem &fs = FileSystem::getInstance();
    std::string route_path = fs.getRouteRootDir() + fs.separator()
                             + route_name.toStdString();

    QDir routeDir(route_path.c_str());

    if (!routeDir.exists())
    {
        return;
    }

    QString scn_subdir_name = "scenarios";

    if (!routeDir.exists(scn_subdir_name))
    {
        if (!routeDir.mkdir(scn_subdir_name))
        {
            return;
        }
    }

    std::string scenarios_path = route_path + fs.separator()
                                 + scn_subdir_name.toStdString();

    QDir scnDir(scenarios_path.c_str());

    if (!scnDir.exists(STARTUP_SCN_SUBDIR))
    {
        if (!scnDir.mkdir(STARTUP_SCN_SUBDIR))
        {
            return;
        }
    }

    std::string file_path = scenarios_path + fs.separator()
                            + STARTUP_SCN_SUBDIR.toStdString()
                            + fs.separator() + "main.lua";

    QFile file(file_path.c_str());

    if (!file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        return;
    }

    QTextStream stream(&file);    

    for (auto line : scnCode)
    {
        stream << line << "\n";
    }

    file.close();
}
