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
#include <synchapi.h>

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
            this, &MainWindow::onRouteSelection);

    connect(ui->cbTrajectories, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTrajectorySelection);

    connect(ui->lwTrains, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onTrainSelection);

    connect(ui->btnStart, &QPushButton::pressed,
            this, &MainWindow::onStartPressed);

    connect(&simulatorProc, &QProcess::started,
            this, &MainWindow::onSimulatorStarted);

    connect(&simulatorProc, &QProcess::finished,
            this, &MainWindow::onSimulatorFinished);

    connect(&viewerProc, &QProcess::finished,
            this, &MainWindow::onViewerFinished);

    connect(ui->cbDirection, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDirectionSelected);

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

    setCentralWidget(ui->twMain);

    setFocusPolicy(Qt::ClickFocus);

    loadTheme();

    QIcon icon(":/images/images/RRS_logo.png");
    setWindowIcon(icon);
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
void MainWindow::setRouteScreenShot(const QString &path)
{
    QFileInfo info(path);

    if (!info.exists())
    {
        ui->lRouteScreenShot->setText(tr("No screenshot"));
        return;
    }

    QImage image(ui->lRouteScreenShot->width(), ui->lRouteScreenShot->height(), QImage::Format_ARGB32);
    image.load(path);
    ui->lRouteScreenShot->setPixmap(QPixmap::fromImage(image));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startSimulator()
{
    FileSystem &fs = FileSystem::getInstance();
    QString simPath = SIMULATOR_NAME + EXE_EXP;

    QStringList args;
    args << "--train-config=" + selectedTrain;
    args << "--route=" + selectedRouteDirName;
    args << "--traj-name=" + selected_train_position.trajectory_name;

    if (isBackward())
    {
        args << "--direction=-1";
    }
    else
    {
        args << "--direction=1";
    }

    if (ui->cbTrajectories->count() != 0)
    {
        double init_coord = ui->dsbOrdinate->value();
        args << "--init-coord=" + QString("%1").arg(init_coord, 0, 'f', 2);
    }

    simulatorProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    simulatorProc.start(simPath, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startViewer()
{
    FileSystem &fs = FileSystem::getInstance();
    QString viewerPath = VIEWER_NAME + EXE_EXP;

    QStringList args;

    viewerProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    viewerProc.start(viewerPath, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadStations(QString &routeDir)
{
    waypoints.clear();

    QFile file(routeDir + QDir::separator() + "waypoints.conf");

    if (!file.open(QIODevice::ReadOnly))
        return;

    while (!file.atEnd())
    {
        QString line = file.readLine();

        waypoint_t waypoint;

        QTextStream ss(&line);

        ss >> waypoint.name >> waypoint.forward_coord >> waypoint.backward_coord;

        waypoints.push_back(waypoint);
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadTrajectories(QString &routeDir)
{
    QString path = routeDir + QDir::separator() +
                   "topology" + QDir::separator() +
                   + "trajectories";

    QDir traj_dir(path);

    QDirIterator traj_files(traj_dir.path(),
                            QStringList() << "*.traj",
                            QDir::NoDotAndDotDot | QDir::Files);


    while (traj_files.hasNext())
    {
        QString fullpath = traj_files.next();

        QFileInfo file_info(fullpath);

        ui->cbTrajectories->addItem(file_info.baseName());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::isBackward()
{
    switch (ui->cbDirection->currentIndex())
    {
    case 0: return false;

    case 1: return true;
    }

    return false;
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
void MainWindow::onRouteSelection()
{
    size_t item_idx = static_cast<size_t>(ui->lwRoutes->currentRow());

    ui->ptRouteDescription->clear();
    selectedRouteDirName = routes_info[item_idx].route_dir_name;
    ui->ptRouteDescription->appendPlainText(routes_info[item_idx].route_description);

    loadStations(routes_info[item_idx].route_dir_full_path);

    loadTrainPositions(routes_info[item_idx].route_dir_full_path);

    onTrajectorySelection(0);

    setRouteScreenShot(routes_info[item_idx].route_dir_full_path + QDir::separator() + "shotcut.png");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onTrajectorySelection(int index)
{
    if (index < 0)
    {
        return;
    }

    if (isBackward())
    {
        selected_train_position = bwd_train_positions[index];
    }
    else
    {
        selected_train_position = fwd_train_positions[index];
    }

    ui->dsbOrdinate->setValue(selected_train_position.traj_coord);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onTrainSelection()
{
    size_t item_idx = static_cast<size_t>(ui->lwTrains->currentRow());

    ui->ptTrainDescription->clear();
    selectedTrain = trains_info[item_idx].train_config_path;
    ui->ptTrainDescription->appendPlainText(trains_info[item_idx].description);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onStartPressed()
{
    // Check is train selected
    if (selectedTrain.isEmpty())
    {
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
void MainWindow::onSimulatorStarted()
{
    ui->btnStart->setEnabled(false);

    Sleep(500);

    startViewer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    ui->btnStart->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onViewerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    simulatorProc.kill();
    setFocusPolicy(Qt::StrongFocus);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onStationSelected(int index)
{
    size_t idx = static_cast<size_t>(index);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onDirectionSelected(int index)
{
    Q_UNUSED(index)

    ui->cbTrajectories->clear();

    if (index == 0)
    {
        for (auto tp = fwd_train_positions.begin(); tp != fwd_train_positions.end(); ++tp)
        {
            ui->cbTrajectories->addItem((*tp).name);
        }
    }
    else
    {
        for (auto tp = bwd_train_positions.begin(); tp != bwd_train_positions.end(); ++tp)
        {
            ui->cbTrajectories->addItem((*tp).name);
        }
    }
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
    ui->cbTrajectories->clear();
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

        //train_positions.insert(tp.name, tp);

        if (tp.direction > 0)
        {
            fwd_train_positions.push_back(tp);
            ui->cbTrajectories->addItem(tp.name);
            ui->cbTrajectories->setCurrentIndex(0);
        }
        else
        {
            bwd_train_positions.push_back(tp);
        }
    }
}
