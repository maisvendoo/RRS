#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    <osgDB/ReadFile>
#include    <osgDB/WriteFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::string path = "../routes/experimental-polygon/models/sig_k2.dmd";

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(path);

    osgDB::writeNodeFile(*model.get(), "sig_k2.dmd");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

