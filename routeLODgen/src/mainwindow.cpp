#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    <osgDB/ReadFile>
#include    <osgDB/WriteFile>

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered,
            this, &MainWindow::slotOnQuit);

    ui->twFilesList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    for (int i = 1; i < ui->twLODparams->columnCount(); ++i)
    {
        ui->twLODparams->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    FileSystem &fs = FileSystem::getInstance();
    routesRootDir = QString(fs.getRouteRootDir().c_str());

    connect(ui->actionOpenRoute, &QAction::triggered,
            this, &MainWindow::slotOnRouteOpen);

    this->setWindowTitle(tr("RRS routes LOD generator"));
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
void MainWindow::slotOnQuit()
{
    QApplication::quit();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnRouteOpen()
{
    objectRefReader();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnCleanRoute()
{

}

