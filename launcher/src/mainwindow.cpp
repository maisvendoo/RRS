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
#include    "ui_mainwindow.h"

#include    <QPushButton>
#include    <QDir>
#include    <QDirIterator>
#include    <QStringList>
#include    <QComboBox>
#include    <QSpinBox>
#include    <QDoubleSpinBox>
#include    <QTextStream>
// #include <thread>
// #include <synchapi.h>



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

    init();

    connect(ui->lwRoutes, &QListWidget::itemSelectionChanged,
            this, &MainWindow::slotRouteSelection);

    connect(ui->lwTrains, &QListWidget::itemSelectionChanged,
            this, &MainWindow::slotTrainSelection);

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

    connect(ui->spWidth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spHeight, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbFullScreen, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbDoubleBuffer, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->cbWindowDecoration, QOverload<int>::of(&QCheckBox::stateChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->dspFovY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, QOverload<double>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->dspNear, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, QOverload<double>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->dspFar, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, QOverload<double>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spViewDist, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->spScreenNumber, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&MainWindow::slotChangedGraphSetting));

    connect(ui->pbCancel, &QPushButton::released, this, &MainWindow::slotCancelGraphSettings);
    connect(ui->pbApply, &QPushButton::released, this, &MainWindow::slotApplyGraphSettings);

    connect(ui->pbAddTrain, &QPushButton::released, this, &MainWindow::slotAddActiveTrain);
    connect(ui->pbDeleteTrain, &QPushButton::released, this, &MainWindow::slotDeleteActiveTrain);

    connect(ui->twActiveTrains, &QTableWidget::cellChanged, this, &MainWindow::slotActiveTrainCellChanged);

    setCentralWidget(ui->twMain);

    setFocusPolicy(Qt::ClickFocus);

    loadTheme();

    ui->twActiveTrains->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twActiveTrains->horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
    ui->twActiveTrains->horizontalHeader()->setSectionsClickable(false);
    ui->twActiveTrains->verticalHeader()->setDefaultSectionSize(18);
    ui->twActiveTrains->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twActiveTrains->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twActiveTrains->setCornerButtonEnabled(false);
    ui->twActiveTrains->verticalHeader()->setVisible(false);

    QIcon icon(":/images/images/RRS_logo.png");
    setWindowIcon(icon);

    ui->pbAddTrain->setEnabled(false);
    ui->pbDeleteTrain->setEnabled(false);
    ui->pbStartViewer->setEnabled(false);
    ui->pbStartMap->setEnabled(false);
    ui->pbStartServer->setEnabled(false);
    is_start_button_to_stop_server = false;
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
void MainWindow::startSimulator()
{
    if (selectedRouteDirName.isEmpty() || active_trains.empty())
    {
        return;
    }

    FileSystem &fs = FileSystem::getInstance();
    QString simPath = SIMULATOR_NAME + EXE_EXP;

    QStringList args;

    QString selected_trains = "";
    QString traj_names = "";
    QString directions = "";
    QString init_coords = "";

    for (auto at = active_trains.begin(); at != active_trains.end(); ++at)
    {
        selected_trains += (*at).train_info.train_config_path;
        traj_names += (*at).train_position.trajectory_name;
        directions += QString("%1").arg((*at).train_position.direction);
        init_coords += QString("%1").arg((*at).train_position.traj_coord, 0, 'f', 2);

        if (at != active_trains.end() - 1)
        {
            selected_trains += ",";
            traj_names += ",";
            directions += ",";
            init_coords += ",";
        }
    }

    args << "--train-config=" + selected_trains;
    args << "--route=" + selectedRouteDirName;
    args << "--traj-name=" + traj_names;
    args << "--direction=" + directions;
    args << "--init-coord=" + init_coords;

    simulatorProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    simulatorProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + simPath, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startViewer()
{
    FileSystem &fs = FileSystem::getInstance();
    QString viewerPath = VIEWER_NAME + EXE_EXP;

    viewerProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    // viewerProc.setStandardOutputFile("../logs/viewer-start.log");
    viewerProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + viewerPath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startMap()
{
    FileSystem &fs = FileSystem::getInstance();
    QString mapPath = ROUTE_MAP_NAME + EXE_EXP;

    mapProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    mapProc.start(QString::fromStdString(fs.getBinaryDir()) + '/' + mapPath);
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
    size_t item_idx = static_cast<size_t>(ui->lwRoutes->currentRow());

    ui->ptRouteDescription->clear();
    selectedRouteDirName = routes_info[item_idx].route_dir_name;
    ui->ptRouteDescription->appendPlainText(routes_info[item_idx].route_description);

    loadTrainPositions(routes_info[item_idx].route_dir_full_path);

    ui->pbAddTrain->setEnabled(ui->lwTrains->currentRow() >= 0);

    if (active_trains.empty())
    {
        return;
    }

    updateActiveTrains();
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
void MainWindow::slotStartServerPressed()
{
    // Check button is stop running server mode
    if (is_start_button_to_stop_server)
    {
        //ui->pbStartServer->setEnabled(false);
        simulatorProc.kill();
        return;
    }

    // Check is route selected
    if (selectedRouteDirName.isEmpty())
    {
        return;
    }

    // Check are active trains selected
    if (active_trains.empty())
    {
        return;
    }

    //ui->pbStartServer->setEnabled(false);
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

    // std::this_thread::sleep_for(std::chrono::milliseconds(500));

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
void MainWindow::slotAddActiveTrain()
{
    if (ui->lwRoutes->currentRow() < 0)
    {
        return;
    }

    int train_idx = ui->lwTrains->currentRow();
    if (train_idx < 0)
    {
        return;
    }

    QTableWidget *tt = ui->twActiveTrains;

    active_train_t at;
    at.train_info = trains_info[train_idx];

    int rowIdx = tt->rowCount();
    tt->insertRow(rowIdx);
    tt->setItem(rowIdx, 0, new QTableWidgetItem(at.train_info.train_title));

    QComboBox *dir = new QComboBox(this);
    dir->addItem(tr("Forward"));
    dir->addItem(tr("Backward"));
    dir->setCurrentIndex(0);
    tt->setCellWidget(rowIdx, 1, dir);

    QComboBox *waypoints = new QComboBox(this);
    tt->setCellWidget(rowIdx, 2, waypoints);

    QDoubleSpinBox *dist = new QDoubleSpinBox(this);
    dist->setMaximum(40000000.0);
    dist->setDecimals(2);
    dist->setAlignment(Qt::AlignRight);
    tt->setCellWidget(rowIdx, 3, dist);

    for (auto tp = fwd_train_positions.begin(); tp != fwd_train_positions.end(); ++tp)
    {
        waypoints->addItem((*tp).name);
    }
    at.train_position = fwd_train_positions[waypoints->currentIndex()];
    dist->setValue(fwd_train_positions[waypoints->currentIndex()].traj_coord);

    active_trains.push_back(at);

    tt->selectRow(rowIdx);

    if (!is_start_button_to_stop_server)
        ui->pbStartServer->setEnabled(true);

    ui->pbDeleteTrain->setEnabled(true);

    connect(dir, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::slotActiveTrainDirectionChange);

    connect(waypoints, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::slotActiveTrainTrajectoryChange);

    connect(dist, &QDoubleSpinBox::valueChanged, this, &MainWindow::slotTrainCoordValueChanged);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDeleteActiveTrain()
{
    QTableWidget *tt = ui->twActiveTrains;

    QModelIndexList selection = tt->selectionModel()->selectedRows();

    for (int i = 0; i < selection.count(); ++i)
    {
        QModelIndex index = selection.at(i);
        tt->removeRow(index.row());
        active_trains.erase(active_trains.begin() + index.row());
    }

    if (active_trains.empty())
    {
        if (!is_start_button_to_stop_server)
            ui->pbStartServer->setEnabled(false);

        ui->pbDeleteTrain->setEnabled(false);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotActiveTrainCellChanged(int row, int column)
{
    int r = row;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotActiveTrainDirectionChange(int idx)
{
    int train_idx = ui->twActiveTrains->currentRow();

    QComboBox *dir = dynamic_cast<QComboBox *>(ui->twActiveTrains->cellWidget(train_idx, 1));
    QComboBox *waypoints = dynamic_cast<QComboBox *>(ui->twActiveTrains->cellWidget(train_idx, 2));
    QDoubleSpinBox *dist = dynamic_cast<QDoubleSpinBox *>(ui->twActiveTrains->cellWidget(train_idx, 3));

    waypoints->clear();
    if (dir->currentIndex() == 0)
    {
        for (auto tp = fwd_train_positions.begin(); tp != fwd_train_positions.end(); ++tp)
        {
            waypoints->addItem((*tp).name);
        }

        if (waypoints->count() != 0)
        {
            waypoints->setCurrentIndex(0);
            active_trains[train_idx].train_position = fwd_train_positions[waypoints->currentIndex()];
            dist->setValue(fwd_train_positions[waypoints->currentIndex()].traj_coord);
        }
    }
    else
    {
        for (auto tp = bwd_train_positions.begin(); tp != bwd_train_positions.end(); ++tp)
        {
            waypoints->addItem((*tp).name);
        }

        if (waypoints->count() != 0)
        {
            waypoints->setCurrentIndex(0);
            active_trains[train_idx].train_position = bwd_train_positions[waypoints->currentIndex()];
            dist->setValue(bwd_train_positions[waypoints->currentIndex()].traj_coord);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotActiveTrainTrajectoryChange(int idx)
{
    int train_idx = this->getSelectedActiveTrainIndex();

    if (train_idx < 0)
        return;

    QComboBox *dir = dynamic_cast<QComboBox *>(ui->twActiveTrains->cellWidget(train_idx, 1));
    QDoubleSpinBox *dist = dynamic_cast<QDoubleSpinBox *>(ui->twActiveTrains->cellWidget(train_idx, 3));

    train_position_t tp;

    if (idx < 0)
        return;

    if (dir->currentIndex() == 0)
    {
        tp = fwd_train_positions[idx];
    }
    else
    {
        tp = bwd_train_positions[idx];
    }

    active_trains[train_idx].train_position = tp;
    dist->setValue(tp.traj_coord);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTrainCoordValueChanged(double value)
{
    int rowIdx = ui->twActiveTrains->currentRow();

    if (rowIdx < 0)
    {
        return;
    }

    active_trains[rowIdx].train_position.traj_coord = value;
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
const   QString MainWindow::NOTIFY_LEVEL = "NofifyLevel";
const   QString MainWindow::VIEW_DIST = "ViewDistance";

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

    ui->spScreenNumber->setValue(findSetting(SCREEN_NUM, fd_list).second.toInt());
    ui->dspFovY->setValue(findSetting(FOV_Y, fd_list).second.toDouble());
    ui->dspNear->setValue(findSetting(ZNEAR, fd_list).second.toDouble());
    ui->dspFar->setValue(findSetting(ZFAR, fd_list).second.toDouble());

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

    findSetting(ZFAR, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(ZFAR, ui->dspFar->value());

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
void MainWindow::loadTrainPositions(const QString &routeDir)
{
    fwd_train_positions.clear();
    bwd_train_positions.clear();

    QString path = routeDir + QDir::separator() +
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
            fwd_train_positions.push_back(tp);
        }
        else
        {
            bwd_train_positions.push_back(tp);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int MainWindow::getSelectedActiveTrainIndex()
{
    QModelIndexList selection = ui->twActiveTrains->selectionModel()->selectedRows();

    if (selection.empty())
        return -1;

    QModelIndex index = *(selection.end() - 1);

    return index.row();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateActiveTrains()
{
    for (int i = 0; i < ui->twActiveTrains->rowCount(); ++i)
    {
        QComboBox *dir = dynamic_cast<QComboBox *>(ui->twActiveTrains->cellWidget(i, 1));
        QComboBox *waypoints = dynamic_cast<QComboBox *>(ui->twActiveTrains->cellWidget(i, 2));
        QDoubleSpinBox *dist = dynamic_cast<QDoubleSpinBox *>(ui->twActiveTrains->cellWidget(i, 3));

        waypoints->clear();
        if (dir->currentIndex() == 0)
        {
            for (auto tp = fwd_train_positions.begin(); tp != fwd_train_positions.end(); ++tp)
            {
                waypoints->addItem((*tp).name);
            }

            if (waypoints->count() != 0)
            {
                waypoints->setCurrentIndex(0);
                active_trains[i].train_position = fwd_train_positions[waypoints->currentIndex()];
                dist->setValue(fwd_train_positions[waypoints->currentIndex()].traj_coord);
            }
        }
        else
        {
            for (auto tp = bwd_train_positions.begin(); tp != bwd_train_positions.end(); ++tp)
            {
                waypoints->addItem((*tp).name);
            }

            if (waypoints->count() != 0)
            {
                waypoints->setCurrentIndex(0);
                active_trains[i].train_position = bwd_train_positions[waypoints->currentIndex()];
                dist->setValue(bwd_train_positions[waypoints->currentIndex()].traj_coord);
            }
        }
    }
}
