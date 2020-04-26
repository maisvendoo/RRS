#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    "filesystem.h"
#include    "CfgEditor.h"
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
    connect(ui->pbConvert, &QPushButton::released, this, &MainWindow::slotConvert);
    connect(&pathconvProc, QOverload<int>::of(&QProcess::finished), this, &MainWindow::slotIsPathconvFinished);
    connect(&profconvProc, QOverload<int>::of(&QProcess::finished), this, &MainWindow::slotIsProfconvFinished);
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
    QFile file(routeDir + QDir::separator() + "route-type");

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

    editor.openFileForWrite(routeDir + QDir::separator() + "description.xml");

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
void MainWindow::startPathConverter(QString routeDir)
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
void MainWindow::startProfConverter(QString routeDir)
{
    FileSystem &fs = FileSystem::getInstance();
    QString profconv_path = PROFCONV + EXE_EXP;

    QStringList args;
    args << "--route=" + routeDir;

    profconvProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    profconvProc.start(profconv_path, args);
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

    QDir dir(routesRootDir);
    QString relPath = dir.relativeFilePath(routeDir);

    ui->lCurrentRouteDir->setText(relPath);
    ui->lStatus->setText(tr("Route opened succesfully"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConvert()
{
    if (routeDir.isEmpty())
    {
        ui->lStatus->setText(tr("Error: route is't loaded. Please choose route"));
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
        ui->lStatus->setText(tr("Error: route-type file is't created"));
        return;
    }

    if (!createDescriptionFile(ui->leRouteTitle->text(), ui->teRouteDescription->toPlainText()))
    {
        ui->lStatus->setText(tr("Error: description file is't created"));
        return;
    }

    startPathConverter(routeDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsPathconvFinished(int error_code)
{
    Q_UNUSED(error_code)

    startProfConverter(routeDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotIsProfconvFinished(int error_code)
{
    Q_UNUSED(error_code)

    ui->lStatus->setText(tr("OK: conversion complete"));
}
