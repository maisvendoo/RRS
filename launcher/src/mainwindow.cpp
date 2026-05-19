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
#include    "find-settings.h"

#include    "platform.h"
#include    "styles.h"

#include    "pdfviewer.h"

#include    <system-diagnostic.h>
#include    <graphsettingswindow.h>

const   QString MainWindow::STARTUP_SCN_SUBDIR = "startup";
const   QString MainWindow::AUTO_START_VIEWER = "AutoStartViewer";
const   QString MainWindow::AUTO_START_ROUTE_MAP = "AutoStartRouteMap";

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

    connect(ui->pbAddTrain, &QPushButton::released, this, &MainWindow::slotAddActiveTrain);
    connect(ui->pbDeleteTrain, &QPushButton::released, this, &MainWindow::slotDeleteActiveTrain);

    connect(ui->pbSaveAsScenario, &QPushButton::released, this, &MainWindow::slotSaveTrainsConfigAsScenario);

    setCentralWidget(ui->twMain);

    setFocusPolicy(Qt::ClickFocus);

    loadSettingsGUI();

    loadConfig();

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

    // Предстартовая диагностика GPU
    gpuDiagnostics();

    connect(ui->actionGraphics_settings, &QAction::triggered, this, [this](){
        graphSettingsWindow->show();
    });

    // Окно скрывается, а не удаляется
    helpWindow->setAttribute(Qt::WA_DeleteOnClose, false);

    createToolsMenu();

    createHelpMenu();

    connect(ui->cbAutostartViewer, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState){
        applyOptions(fd_options, ui);
        saveOptions(fd_options);
    });

    connect(ui->cbAutostartMap, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState){
        applyOptions(fd_options, ui);
        saveOptions(fd_options);
    });
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

        routes_info.emplace_back(std::move(route_info));
    }

    for (const route_info_t& route_info : routes_info)
    {
        ui->lwRoutes->addItem(route_info.route_title);
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

            // Пропускаем startup-сценарий, чтобы юзеру глаза не мозолил
            if (scn.scenario_name == STARTUP_SCN_SUBDIR)
            {
                continue;
            }

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
void MainWindow::reloadScenariosList()
{
    ui->cbScenario->clear();
    ui->cbScenario->addItem(tr("<Not_selected>"));

    for (auto& sc : routes_info[selected_route_idx].scenarios)
    {
        ui->cbScenario->addItem(sc.scenario_name);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::showTrainsConfigTip()
{
    if (trainsConfigTip != nullptr)
    {
        return;
    }

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(ui->frame->layout());

    if (layout == nullptr)
    {
        layout = new QVBoxLayout(this);
        ui->frame->setLayout(layout);
    }

    layout->setAlignment(Qt::AlignCenter);

    trainsConfigTip = new QLabel(tr("Trains positions are defined in scenario"), ui->frame);
    trainsConfigTip->setAlignment(Qt::AlignCenter);

    layout->addWidget(trainsConfigTip);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::hideTrainsConfigsTip()
{
    if (trainsConfigTip == nullptr)
    {
        return;
    }

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(ui->frame->layout());

    if (layout == nullptr)
    {
        return;
    }

    layout->removeWidget(trainsConfigTip);
    trainsConfigTip->deleteLater();
    trainsConfigTip = nullptr;

    layout->deleteLater();
    layout = nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString getCenteredHtml(QString msg, QString color = "#ff6b6b")
{
    if (color.isEmpty())
        return QString("<p style='text-align: center;'>%1</p>").arg(msg);

    return QString("<p style='text-align: center; color: %1;'>%2</p>")
        .arg(color, msg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::gpuDiagnostics()
{
    // Получаем список GPU
    auto status = getInfoGPUs(gpus_info);

    start_viewer_allowed = false;
    ui->tbLogGPU->clear();

    switch (status)
    {
    case GPU_STATE_VULKAN_LOADER_NOT_FOUND_ERROR:
        {
        ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: missing Vulkan loader in your system. Check that you have lastest version of driver for your GPU from offcial vendor site.")));
            break;
        }
    case GPU_STATE_VK_INSTANCE_ERROR:
        {
            ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: can not create vkInstance. Check that you have lastest version of driver for your GPU from offcaial vendor site.")));
            break;
        }

    case GPU_STATE_VK_ENUM_PHYSICAL_DEVICE_ERROR:
        {
            ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: GPU driver error, can't get information about GPUs. Check that you have lastest version of driver for your GPU from offcial vendor site")));
            break;
        }

    case GPU_STATE_NO_CAPABLE_DEVICES_ERROR:
        {
            ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: not find GPU devices capable with Vulkan API.")));
            break;
        }

    case GPU_STATE_GET_DEVICES_LIST_ERROR:
        {
            ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: GPU driver error, can't get GPUs list. Check that you have lastest version of driver for your GPU from offcial vendor site")));
            break;
        }

    case GPU_STATE_READY:
        {
            if (gpus_info.empty())
            {
                break;
            }

            // последний рубеж - проверка версии ОС. На тот случай, когда устройство
            // надежно детектируется как Vulkan-совместимое, но на заведомо совместимом обрудовании
            // не запускает и падает вьювер, а драйверы не хотят устанавливаться
            // жалуясь на версию ОС.
            // TODO: протестировать это!!! иначе огребем от юзеров по полной!
            size_t valid_gpus_count = 0;
            size_t invalid_gpus_count = 0;
            std::vector<size_t> devices_with_problems;

            for (size_t i = 0; i < gpus_info.size(); ++i)
            {
                if (checkOperationSystemVersion(gpus_info[i].vendorID, winver, gpus_info[i].nameOS))
                {
                    ++valid_gpus_count;
                }
                else
                {
                    devices_with_problems.push_back(i);
                }
            }

            if (valid_gpus_count == 0)
            {
                ui->tbLogGPU->setHtml(getCenteredHtml(tr("Start graphics client is impossible: You operation system %1 is not capable with modern direvers of all your devices. Please, update you operation system").arg(gpus_info[0].nameOS)));
                return;
            }

            if (!devices_with_problems.empty())
            {
                QString devList = "";
                for (auto i : devices_with_problems)
                {
                    devList += gpus_info[i].deviceName + ", ";
                    ui->tbLogGPU->setHtml(getCenteredHtml(tr("WARNING: You devices ") + devList + tr(" may have a problems with start graphics client, becourse modern drivers for them not capable with your OS version"), "#ffff00"));
                }
            }

            start_viewer_allowed = true;
            graphSettingsWindow->setSettingsGPU(gpus_info);

            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::createHelpMenu()
{
    FileSystem &fs = FileSystem::getInstance();
    auto docsDir = fs.getDocsDir();

    QDir docs(QString(docsDir.c_str()));
    QDirIterator docs_files(docs.path(),
                            QStringList() << "*.pdf",
                            QDir::NoDotAndDotDot | QDir::Files);

    while (docs_files.hasNext())
    {
        QString fullPath = docs_files.next();
        QFileInfo fileInfo(fullPath);

        //train_info.train_config_path = fileInfo.baseName();

        QAction *action = new QAction(fileInfo.baseName());
        ui->menuHelp->addAction(action);

        connect(action, &QAction::triggered, this, [this, fullPath, fileInfo]{

            if (fullPath.isEmpty())
            {
                return;
            }

            auto *viewer = new PdfViewer(helpWindow);

            if (!viewer->load(fullPath))
            {
                delete viewer;
                return;
            }

            if (auto* old = helpWindow->centralWidget())
            {
                old->deleteLater();
            }

            helpWindow->setCentralWidget(viewer);
            helpWindow->setWindowTitle(QString("%2 | %1 страниц").arg(viewer->pageCount()).arg(fileInfo.baseName()));
            helpWindow->show();
            helpWindow->resize(900, 1000);

            QTimer::singleShot(0, viewer, [this, viewer]() {
                viewer->adjustWindowToPageWidth(0); // 0 = первая страница
                this->centerWindow(helpWindow);
            });
        });
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::createToolsMenu()
{
    CfgReader cfg;
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_dir = fs.getConfigDir();
    std::string cfg_path = fs.combinePath(cfg_dir, "launcher.xml");

    if (!cfg.load(QString(cfg_path.c_str())))
    {
        return;
    }

    auto secNode = cfg.getFirstSection("Tool");

    QString binPath = QDir::toNativeSeparators(QString(fs.getBinaryDir().c_str()));

    while (!secNode.isNull())
    {
        QString toolName;
        cfg.getString(secNode, "Name", toolName);

        if (toolName.isEmpty())
        {
            continue;
        }

        QString toolPath = binPath + QDir::separator() + toolName + EXE_EXP;

        QFile toolFile(toolPath);

        if (!toolFile.exists())
        {
            continue;
        }

        QProcess *proc = new QProcess(this);
        proc->setWorkingDirectory(binPath);
        proc->setProgram(toolPath);

        QString args_str;
        cfg.getString(secNode, "CommandLine", args_str);
        QStringList args = args_str.split(' ');

        if (!args.empty())
        {
            proc->setArguments(args);
        }

        QString description;
        cfg.getString(secNode, "Description", description);

        if (description.isEmpty())
        {
            description = toolName;
        }

        toolProcs.push_back(proc);

        QAction *action = new QAction(description);

        connect(action, &QAction::triggered, this, [this](){

            QAction *ac = qobject_cast<QAction *>(sender());

            if (ac == nullptr)
            {
                return;
            }

            int idx = ui->menuTools->actions().indexOf(ac);

            if (idx >= 0 && idx < toolProcs.size())
            {
                if (toolProcs[idx] != nullptr && toolProcs[idx]->state() != QProcess::Running)
                {
                    toolProcs[idx]->start();
                }
            }
        });

        ui->menuTools->addAction(action);

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::centerWindow(QWidget *window)
{
    if (!window || !window->parentWidget())
    {
        return;
    }

    QRect parentRect = window->parentWidget()->geometry();
    QRect myRect = window->geometry();
    int x = parentRect.x() + (parentRect.width() - myRect.width()) / 2;
    int y = parentRect.y() + (parentRect.height() - myRect.height()) / 2;
    window->move(x, y);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateOptions(FieldsDataList &fd_options)
{
    findSetting(AUTO_START_VIEWER, fd_options).second.toBool() ?
        ui->cbAutostartViewer->setCheckState(Qt::CheckState::Checked) :
        ui->cbAutostartViewer->setCheckState(Qt::CheckState::Unchecked);

    findSetting(AUTO_START_ROUTE_MAP, fd_options).second.toBool() ?
        ui->cbAutostartMap->setCheckState(Qt::CheckState::Checked) :
        ui->cbAutostartMap->setCheckState(Qt::CheckState::Unchecked);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applyOptions(FieldsDataList &fd_options, Ui::MainWindow *ui)
{
    int idx = 0;
    findSetting(AUTO_START_VIEWER, fd_options, idx);
    fd_options[idx] = QPair<QString, QVariant>(AUTO_START_VIEWER, static_cast<int>(ui->cbAutostartViewer->checkState() == Qt::CheckState::Checked));

    findSetting(AUTO_START_ROUTE_MAP, fd_options, idx);
    fd_options[idx] = QPair<QString, QVariant>(AUTO_START_ROUTE_MAP, static_cast<int>(ui->cbAutostartMap->checkState() == Qt::CheckState::Checked));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadActiveTrainsList()
{
    // TODO: Разные типы с size_t
    if ((selected_route_idx < 0) || (selected_route_idx >= routes_info.size()))
    {
        return;
    }

    reloadScenariosList();

    slotUpdateActiveTrains();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::saveOptions(FieldsDataList &fd_options)
{
    CfgEditor editor;

    editor.editFile(settings_path, "Launcher", fd_options);
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

    if (selected_scenario_idx >= 0)
    {
        args << "--scenario=" + routes_info[selected_route_idx].scenarios[selected_scenario_idx].scenario_name;
    }
    else
    {
        auto scnCode = createTmpScenarioCode(active_trains);
        createScenario(selectedRouteDirName, scnCode);
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
    if (!start_viewer_allowed)
    {
        return;
    }

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
void MainWindow::loadSettingsGUI()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_dir = fs.getConfigDir();
    std::string cfg_path = fs.combinePath(cfg_dir, "gui-settings.xml");

    CfgReader cfg;

    if ( cfg.load(QString(cfg_path.c_str())) )
    {
        QString secName = "GUISettings";
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
void MainWindow::loadConfig()
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

        cfg.getInt(secName, "MinWinver", winver.majorVer);
        cfg.getInt(secName, "MinWinBuild_NVIDIA", winver.buildNvidia);
        cfg.getInt(secName, "MinWinBuild_AMD", winver.buildAMD);
        cfg.getInt(secName, "MinWinBuild_INTEL", winver.buildIntel);

        settings_path = QString(cfg_path.c_str());

        bool is_auto_start_viewer = false;
        cfg.getBool(secName, AUTO_START_VIEWER, is_auto_start_viewer);
        fd_options.append(QPair<QString, QVariant>(AUTO_START_VIEWER, is_auto_start_viewer));

        bool is_auto_start_route_map = false;
        cfg.getBool(secName, AUTO_START_ROUTE_MAP, is_auto_start_route_map);
        fd_options.append(QPair<QString, QVariant>(AUTO_START_ROUTE_MAP, is_auto_start_route_map));

        updateOptions(fd_options);
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

    ui->cbScenario->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDeleteActiveTrain()
{
    if (tbActiveTrains->count() <= 0)
    {
        return;
    }

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

    if (active_trains.size() == 0)
    {
        ui->cbScenario->setEnabled(true);
    }
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
        // TODO: Разные типы с size_t
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
    (void)reset_start_config;

    active_trains.clear();

    int active_trains_count = tbActiveTrains->count();
    if (active_trains_count <= 0)
    {
        if (!is_start_button_to_stop_server)
            ui->pbStartServer->setEnabled(false);

        ui->pbDeleteTrain->setEnabled(false);
        ui->cbScenario->setEnabled(true);

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
    Q_UNUSED(exitStatus);

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
    Q_UNUSED(exitStatus)

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
    Q_UNUSED(exitStatus)

    if (simulatorProc.state() != QProcess::NotRunning)
        ui->pbStartMap->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAdditionalProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

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
void MainWindow::slotOnScenarioSelection(int cur_idx)
{
    ui->tbScenarioDescription->clear();
    selected_scenario_idx = -1;

    if (cur_idx <= 0)
    {
        ui->pbStartServer->setEnabled(false);
        hideTrainsConfigsTip();
        ui->pbAddTrain->setEnabled(true);
        return;
    }

    QString desc = routes_info[selected_route_idx].scenarios[cur_idx - 1].scenario_description;
    ui->tbScenarioDescription->setMarkdown(desc);

    selected_scenario_idx = cur_idx - 1;

    ui->pbStartServer->setEnabled(true);

    showTrainsConfigTip();
    ui->pbAddTrain->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveTrainsConfigAsScenario()
{
    if (ui->leScnName->text().isEmpty())
    {
        return;
    }

    QStringList scnCode;
    scnCode.clear();

    if (ui->ckbSaveDateTime->isChecked())
    {
        scnCode.append(createLuaSetDate(ui->dteStartDate));
        scnCode.append(createLuaSetTime(ui->dteStartTime));
        scnCode.append("\n");
    }

    slotUpdateActiveTrains();

    scnCode.append(createTmpScenarioCode(active_trains));

    createScenario(selectedRouteDirName, scnCode, ui->leScnName->text());

    // TODO: Разные типы с size_t
    if (selected_route_idx < 0 || selected_route_idx >= routes_info.size())
    {
        return;
    }

    auto ri = &routes_info[selected_route_idx];

    loadScenarios(*ri);

    reloadScenariosList();

    ui->pbStartServer->setEnabled(!active_trains.empty());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::createLuaSetDate(QDateEdit *dateEdit)
{
    QString setDate = "";

    setDate = QString("setDate(\"%1.%2.%3\")")
                  .arg(dateEdit->dateTime().date().day(), 2, 10, QChar('0'))
                  .arg(dateEdit->dateTime().date().month(), 2, 10, QChar('0'))
                  .arg(dateEdit->dateTime().date().year(), 4, 10, QChar('0'));

    return setDate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::createLuaSetTime(QTimeEdit *timeEdit)
{
    QString setTime = "";

    setTime = QString("setTime(\"%1:%2:%3\")")
                  .arg(timeEdit->dateTime().time().hour(), 2, 10, QChar('0'))
                  .arg(timeEdit->dateTime().time().minute(), 2, 10, QChar('0'))
                  .arg(timeEdit->dateTime().time().second(), 2, 10, QChar('0'));

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

    if (at.is_autopilot_on)
    {
        setTrainCode.append(varName + QString(".auto = true"));
    }

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
void MainWindow::createScenario(const QString &route_name,
                                const QStringList &scnCode,
                                const QString scenario_name)
{
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

    if (!scnDir.exists(scenario_name))
    {
        if (!scnDir.mkdir(scenario_name))
        {
            return;
        }
    }

    std::string file_path = scenarios_path + fs.separator()
                            + scenario_name.toStdString()
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
