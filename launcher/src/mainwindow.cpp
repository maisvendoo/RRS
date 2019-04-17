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
#include    <QTextStream>

#include    "filesystem.h"
#include    "CfgReader.h"

#include    "platform.h"

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

    connect(ui->lwTrains, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onTrainSelection);

    connect(ui->btnStart, &QPushButton::pressed,
            this, &MainWindow::onStartPressed);

    connect(&simulatorProc, &QProcess::started,
            this, &MainWindow::onSimulatorStarted);

    connect(&simulatorProc, QOverload<int>::of(&QProcess::finished),
            this, &MainWindow::onSimulatorFinished);

    connect(&viewerProc, QOverload<int>::of(&QProcess::finished),
            this, &MainWindow::onViewerFinished);    

    setCentralWidget(ui->twMain);

    setFocusPolicy(Qt::ClickFocus);
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
        route_info.route_dir = route_dirs.next();

        CfgReader cfg;

        if (cfg.load(route_info.route_dir + QDir::separator() + "description.xml"))
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
    if (path.isEmpty())
    {
        ui->lRouteScreenShot->setText("Нет скриншота");
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
    args << "--route=" + selectedRoutePath;

    if (ui->cbStations->count() != 0)
    {
        size_t i = static_cast<size_t>(ui->cbStations->currentIndex());
        double init_coord = waypoints[i].railway_coord / 1000.0;
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
    args << "--route" << selectedRoutePath;
    args << "--train" << selectedTrain;

    viewerProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    viewerProc.start(viewerPath, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadStations(QString &routeDir)
{
    ui->cbStations->clear();
    waypoints.clear();

    QFile file(routeDir + QDir::separator() + "waypoints.conf");

    if (!file.open(QIODevice::ReadOnly))
        return;

    while (!file.atEnd())
    {
        QString line = file.readLine();

        waypoint_t waypoint;

        QTextStream ss(&line);

        ss >> waypoint.name >> waypoint.railway_coord;

        ui->cbStations->addItem(waypoint.name);
        waypoints.push_back(waypoint);
    }

    ui->cbStations->setCurrentIndex(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onRouteSelection()
{
    size_t item_idx = static_cast<size_t>(ui->lwRoutes->currentRow());

    ui->ptRouteDescription->clear();
    selectedRoutePath = routes_info[item_idx].route_dir;
    ui->ptRouteDescription->appendPlainText(routes_info[item_idx].route_description);

    loadStations(selectedRoutePath);

    setRouteScreenShot(selectedRoutePath + QDir::separator() + "shotcut.png");
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
    if (selectedRoutePath.isEmpty())
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

    startViewer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onSimulatorFinished(int exitCode)
{
    Q_UNUSED(exitCode)

    ui->btnStart->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onViewerFinished(int exitCode)
{
    Q_UNUSED(exitCode)

    simulatorProc.kill();
    setFocusPolicy(Qt::StrongFocus);
}
