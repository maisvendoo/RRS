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

    const FileSystem& fs = FileSystem::getInstance();
    routesRootDir = QString(fs.getRouteRootDir().c_str());

    connect(ui->pbOpenRoute, &QPushButton::released, this, &MainWindow::slotOpenRoute);
    connect(ui->pbSelectOutputPath, &QPushButton::released, this, &MainWindow::slotSelectOutputPath);
    connect(ui->pbConvert, &QPushButton::released, this, &MainWindow::slotConvert);
    connect(&pathconvProc, &QProcess::finished, this, &MainWindow::slotIsPathconvFinished);
    connect(&profconvProc, &QProcess::finished, this, &MainWindow::slotIsProfconvFinished);
    connect(&dmd2gltfProc, &QProcess::finished, this, &MainWindow::slotIsDmd2gltfFinished);
    connect(&dmd2gltfProc, &QProcess::readyReadStandardError, this, &MainWindow::slotSubProcessReadyReadStandardError);

    connect(ui->pbCheckTopology, &QPushButton::released, this, &MainWindow::slotCheckTopology);
    connect(ui->pbTransformRoute, &QPushButton::released, this, &MainWindow::slotTransformRoute);
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
    flist.append(QPair<QString, double>("Latitude", latitude));
    flist.append(QPair<QString, double>("Longitude", longitude));

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

        fieldName = "Latitude";
        cfg.getDouble(secName, fieldName, latitude);

        fieldName = "Longitude";
        cfg.getDouble(secName, fieldName, longitude);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startPathConverter()
{
    const FileSystem& fs = FileSystem::getInstance();
    QString pathconv_path = PATHCONV EXE_EXP;

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
    const FileSystem& fs = FileSystem::getInstance();
    QString profconv_path = PROFCONV EXE_EXP;

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
    const FileSystem& fs = FileSystem::getInstance();
    QString dmd2gltf_path = DMD2GLTF EXE_EXP;

    QStringList args;
    args << "--input-route" << routeDir;
    args << "--output-route" << outputDir;
    if (ui->cbOnlyUsedModels->checkState())
    {
        args << "--used-only";
    }
    args << "--lights";
    if (ui->cbCompressTextures->checkState())
    {
        args << "--compress-textures";
    }

    dmd2gltfProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    dmd2gltfProc.start(dmd2gltf_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startTopologyChecker()
{
    const FileSystem& fs = FileSystem::getInstance();
    QString topologycheck_path = TOPOLOGYCHECK EXE_EXP;

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
void MainWindow::startTransformRoute()
{
    const FileSystem& fs = FileSystem::getInstance();
    QString transformroute_path = ROUTETRANSFORM EXE_EXP;

    double delta_x = ui->dsbDeltaX->value();
    double delta_y = ui->dsbDeltaY->value();
    double delta_z = ui->dsbDeltaZ->value();

    QStringList args;
    args << "--input-route" << routeDir;
    args << "--delta-x" << QString("%1").arg(delta_x, 0, 'f', 3);
    args << "--delta-y" << QString("%1").arg(delta_y, 0, 'f', 3);
    args << "--delta-z" << QString("%1").arg(delta_z, 0, 'f', 3);
    if (ui->cbTransformMap->checkState())
    {
        args << "--map-transform";
    }
    if (ui->cbTransformTopology->checkState())
    {
        args << "--topology-transform";
    }

    transformRouteProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    transformRouteProc.start(transformroute_path, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::startParallelGenerator()
{
    const FileSystem& fs = FileSystem::getInstance();
    QString pathconv_path = PARALLELGEN EXE_EXP;

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
    const FileSystem& fs = FileSystem::getInstance();
    QString pathconv_path = SPLINEGEN EXE_EXP;

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
void MainWindow::updateStatus()
{
    subProcessStatus = "";
    ui->lStatus->setText(status);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateStatus(QString new_status, QString new_subprocess_status)
{
    if (status != new_status)
    {
        status = new_status;
        subProcessStatus = "";
        ui->lStatus->setText(status);
    }
    else if (subProcessStatus != new_subprocess_status)
    {
        subProcessStatus = new_subprocess_status;
        ui->lStatus->setText(status + subProcessStatus);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int MainWindow::getDMDConversionPercent(QString sub_status)
{
    QString tmp = sub_status.remove(" ");
    tmp = tmp.remove("(").remove(")");

    auto tokens = tmp.split("/");

    if (tokens.size() < 2)
    {
        return 0;
    }

    double complete = tokens[0].toDouble();
    int total = tokens[1].toInt();

    if (total == 0)
    {
        return 0;
    }

    return qRound(complete * 100.0 / total);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSubProcessReadyReadStandardError()
{
    if (QProcess* p = dynamic_cast<QProcess *>(sender()))
    {
        QString new_subprocess_status = QString::fromUtf8(p->readAllStandardError());

        ui->prbDmdConversion->setValue(getDMDConversionPercent(new_subprocess_status));

        updateStatus(status, new_subprocess_status);
    }
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
        updateStatus(tr("Route is not selected. Please choose route"));
        ui->lCurrentRouteDir->setText("");
        ui->lOutputPath->setText("");

        ui->pbSelectOutputPath->setEnabled(false);
        return;
    }

    ui->pbSelectOutputPath->setEnabled(true);
    updateStatus(tr("Route opened succesfully"));

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
            updateStatus(tr("Route is not selected. Please choose route"));
            ui->lOutputPath->setText("");
            return;
        }
        else
        {
            updateStatus(tr("Output is not selected. Using selected route as output"));
            outputDir = routeDir;
        }
    }
    else
    {
        updateStatus(tr("Output selected succesfully"));
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
        updateStatus(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leRouteTitle->text().isEmpty())
    {
        updateStatus(tr("Error: route title is empty. Please fill it"));
        return;
    }

    if (ui->teRouteDescription->toPlainText().isEmpty())
    {
        updateStatus(tr("Error: route description is empty. Please fill it"));
        return;
    }

    if (!createRouteTypeFile())
    {
        updateStatus(tr("Error: route-type file is not created"));
        return;
    }

    if (!createDescriptionFile(ui->leRouteTitle->text(), ui->teRouteDescription->toPlainText()))
    {
        updateStatus(tr("Error: description file is not created"));
        return;
    }

    updateStatus(tr("Filepath conversion..."));
    startPathConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCheckTopology()
{
    if (routeDir.isEmpty())
    {
        updateStatus(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    startTopologyChecker();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTransformRoute()
{
    if (routeDir.isEmpty())
    {
        updateStatus(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    startTransformRoute();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGenerateParallel()
{
    if (routeDir.isEmpty())
    {
        updateStatus(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leFileParallel->text().isEmpty())
    {
        updateStatus(tr("Error: filename is empty. Please fill it"));
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
        updateStatus(tr("Error: route is not loaded. Please choose route"));
        return;
    }

    if (ui->leFileSpline->text().isEmpty())
    {
        updateStatus(tr("Error: filename is empty. Please fill it"));
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

    updateStatus(tr("Tracks topology conversion..."));
    startProfConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsProfconvFinished(int error_code, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(error_code)

    updateStatus(tr("3d-models conversion..."));
    startDmd2gltfConverter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsDmd2gltfFinished(int error_code, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(error_code)

    updateStatus(tr("OK: conversion complete"));
}
