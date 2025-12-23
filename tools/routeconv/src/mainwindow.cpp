#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    "filesystem.h"
#include    "CfgEditor.h"
#include    "CfgReader.h"
#include    "platform.h"

#include    <QFileDialog>
#include    <QDir>
#include    <QFile>
#include    <QTextStream>
#include    <QPair>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , routesRootDir("")
  , routeDir("")

{
    ui->setupUi(this);

    setWindowTitle("ZDS route converter");

    FileSystem &fs = FileSystem::getInstance();
    routesRootDir = QString(fs.getRouteRootDir().c_str());

    connect(ui->pbOpenRoute, &QPushButton::released, this, &MainWindow::slotOpenRoute);
    connect(ui->pbSelectOutputPath, &QPushButton::released, this, &MainWindow::slotSelectOutputPath);
    connect(ui->pbConvert, &QPushButton::released, this, &MainWindow::slotConvert);
    connect(&pathconvProc, &QProcess::finished, this, &MainWindow::slotIsPathconvFinished);
    connect(&profconvProc, &QProcess::finished, this, &MainWindow::slotIsProfconvFinished);
    connect(&dmd2gltfProc, &QProcess::finished, this, &MainWindow::slotIsDmd2gltfFinished);

    connect(ui->pbCheckTopology, &QPushButton::released, this, &MainWindow::slotCheckTopology);
    connect(ui->pbGenParallel, &QPushButton::released, this, &MainWindow::slotGenerateParallel);
    connect(ui->pbGenSpline, &QPushButton::released, this, &MainWindow::slotGenerateSpline);

    ui->pbSelectOutputPath->setEnabled(false);
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
bool MainWindow::createRouteTypeFile()
{
    QFile file(outputDir + QDir::separator() + "route-type");

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream ss(&file);
        ss << "zds";
        file.close();

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::createDescriptionFile(QString title, QString description)
{
    CfgEditor editor;

    editor.openFileForWrite(outputDir + QDir::separator() + "description.xml");
    editor.setIndentationFormat(-1);

    FieldsDataList flist;
    flist.append(QPair<QString, QString>("Title", title));
    flist.append(QPair<QString, QString>("Description", description));

    editor.writeFile("Route", flist);

    editor.closeFileAfterWrite();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::loadDescriptionFile(QString route_dir)
{
    CfgReader cfg;
    if (cfg.load(route_dir + QDir::separator() + "description.xml"))
    {
        QString secName = "Route";
        QString fieldName = "";
        QString fieldValue = "";

        fieldName = "Title";
        cfg.getString(secName, fieldName, fieldValue);
        ui->leRouteTitle->setText(fieldValue);

        fieldName = "Description";
        cfg.getString(secName, fieldName, fieldValue);
        ui->teRouteDescription->setText(fieldValue);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startPathConverter()
{
    FileSystem &fs = FileSystem::getInstance();
    QString pathconv_path = PATHCONV + EXE_EXP;

    QStringList args;
    args << "--route=" + routeDir;

    pathconvProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    pathconvProc.start(pathconv_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startProfConverter()
{
    FileSystem &fs = FileSystem::getInstance();
    QString profconv_path = PROFCONV + EXE_EXP;

    QStringList args;
    args << "--input-route" << routeDir;
    args << "--output-route" << outputDir;

    profconvProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    profconvProc.start(profconv_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startDmd2gltfConverter()
{
    FileSystem &fs = FileSystem::getInstance();
    QString dmd2gltf_path = DMD2GLTF + EXE_EXP;

    QStringList args;
    args << "--input-route" << routeDir;
    args << "--output-route" << outputDir;
    if (ui->cbOnlyUsedModels->checkState())
    {
        args << "--used-only";
    }
    args << "--lights";

    dmd2gltfProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    dmd2gltfProc.start(dmd2gltf_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startTopologyChecker()
{
    FileSystem &fs = FileSystem::getInstance();
    QString topologycheck_path = TOPOLOGYCHECK + EXE_EXP;

    double curve_min_radius = ui->dsbMinimumRadius->value();

    QStringList args;
    args << "--route" << routeDir;
    args << "--curve" << QString("%1").arg(curve_min_radius, 0, 'f', 2);

    topologyCheckProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    topologyCheckProc.start(topologycheck_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startParallelGenerator()
{
    FileSystem &fs = FileSystem::getInstance();
    QString pathconv_path = PARALLELGEN + EXE_EXP;

    QString filename = ui->leFileParallel->text();
    int trkfile = ui->cbDataParallel->currentIndex() + 1;
    int begin_track = ui->sbTrackBegin->value();
    int end_track = ui->sbTrackEnd->value();
    double bias = ui->dsbParallelOffset->value();

    QStringList args;
    args << "-i" << routeDir;
    args << "-o" << outputDir;
    args << "-f" << filename;
    args << "-t" << QString("%1").arg(trkfile);
    args << "-b" << QString("%1").arg(begin_track);
    args << "-e" << QString("%1").arg(end_track);
    args << "-d" << QString("%1").arg(bias, 0, 'f', 2);

    parallelGenProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    parallelGenProc.start(pathconv_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startSplineGenerator()
{
    FileSystem &fs = FileSystem::getInstance();
    QString pathconv_path = SPLINEGEN + EXE_EXP;

    QString filename = ui->leFileSpline->text();
    int trkfile = ui->cbDataSpline->currentIndex() + 1;
    int track = ui->sbSplineTrack->value();
    double len = ui->sbSplineLength->value();
    double begin_bias = ui->dsbSplineOffsetBegin->value();
    double end_bias = ui->dsbSplineOffsetEnd->value();

    QStringList args;
    args << "-i" << routeDir;
    args << "-o" << outputDir;
    args << "-f" << filename;
    args << "-t" << QString("%1").arg(trkfile);
    args << "-n" << QString("%1").arg(track);
    args << "-l" << QString("%1").arg(len);
    args << "-d" << QString("%1").arg(begin_bias, 0, 'f', 2);
    args << "-e" << QString("%1").arg(end_bias, 0, 'f', 2);

    splineGenProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    splineGenProc.start(pathconv_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenRoute()
{
    routeDir = QFileDialog::getExistingDirectory(this, tr("Open route"),
                                                 routesRootDir,
                                                 QFileDialog::ShowDirsOnly |
                                                 QFileDialog::DontResolveSymlinks);

    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Route is not selected. Please choose route"));
        ui->lCurrentRouteDir->setText("");
        ui->lOutputPath->setText("");

        ui->pbSelectOutputPath->setEnabled(false);
        return;
    }

    ui->pbSelectOutputPath->setEnabled(true);
    ui->lStatus->setText(tr("Route opened succesfully"));

    QDir dir(routesRootDir);
    QString relPath = dir.relativeFilePath(routeDir);
    ui->lCurrentRouteDir->setText(relPath);

    outputDir = routeDir;
    ui->lOutputPath->setText(relPath);

    if (!loadDescriptionFile(routeDir))
    {
        ui->leRouteTitle->setText("");
        ui->teRouteDescription->setText("");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSelectOutputPath()
{
    outputDir = QFileDialog::getExistingDirectory(this, tr("Select converted route path"),
                                                  routesRootDir,
                                                  QFileDialog::ShowDirsOnly |
                                                  QFileDialog::DontResolveSymlinks);

    if (outputDir.isEmpty())
    {
        if (routeDir.isEmpty())
        {
            ui->lStatus->setText(tr("Route is not selected. Please choose route"));
            ui->lOutputPath->setText("");
            return;
        }
        else
        {
            ui->lStatus->setText(tr("Output is not selected. Using selected route as output"));
            outputDir = routeDir;
        }
    }
    else
    {
        ui->lStatus->setText(tr("Output selected succesfully"));
    }

    QDir dir(routesRootDir);
    QString relPath = dir.relativeFilePath(outputDir);
    ui->lOutputPath->setText(relPath);

    if (ui->leRouteTitle->text().isEmpty() && ui->teRouteDescription->toPlainText().isEmpty())
        loadDescriptionFile(outputDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConvert()
{
    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leRouteTitle->text().isEmpty())
    {
        ui->lStatus->setText(tr("Error: route title is empty. Please fill it"));
        return;
    }

    if (ui->teRouteDescription->toPlainText().isEmpty())
    {
        ui->lStatus->setText(tr("Error: route description is empty. Please fill it"));
        return;
    }

    if (!createRouteTypeFile())
    {
        ui->lStatus->setText(tr("Error: route-type file is not created"));
        return;
    }

    if (!createDescriptionFile(ui->leRouteTitle->text(), ui->teRouteDescription->toPlainText()))
    {
        ui->lStatus->setText(tr("Error: description file is not created"));
        return;
    }

    ui->lStatus->setText(tr("Conversion..."));
    startPathConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCheckTopology()
{
    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    startTopologyChecker();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGenerateParallel()
{
    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leFileParallel->text().isEmpty())
    {
        ui->lStatus->setText(tr("Error: filename is empty. Please fill it"));
        return;
    }

    startParallelGenerator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGenerateSpline()
{
    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leFileSpline->text().isEmpty())
    {
        ui->lStatus->setText(tr("Error: filename is empty. Please fill it"));
        return;
    }

    startSplineGenerator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsPathconvFinished(int error_code, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(error_code)

    startProfConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsProfconvFinished(int error_code, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(error_code)

    startDmd2gltfConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsDmd2gltfFinished(int error_code, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(error_code)

    ui->lStatus->setText(tr("OK: conversion complete"));
}
