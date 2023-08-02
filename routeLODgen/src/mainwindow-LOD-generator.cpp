#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    <QDir>
#include    <QFileInfo>

#include    <osgDB/ReadFile>
#include    <osgDB/WriteFile>
#include    <osgUtil/Simplifier>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnLODGenerate()
{
    LOD_count = 0;
    model_it = used_models.begin();
    LODGenTimer.start(100);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnLODGenTimer()
{
    if (LOD_count == used_models.count())
    {
        LODGenTimer.stop();
        return;
    }

    QString model_path = model_it.value();
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(model_path.toStdString());

    for (size_t i = 0; i < LOD_list.size(); ++i)
    {
        QFileInfo model_file(model_path);
        QString path = model_file.path();
        QString ext = model_file.suffix();
        QString baseName = model_file.baseName();

        QString LOD_name = baseName + LOD_list[i].getNameSuphyx() + "." + ext;

        QString LOD_path = path + QDir::separator() + LOD_name;

        osg::ref_ptr<osg::Node> LOD_model = dynamic_cast<osg::Node *>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

        osgUtil::Simplifier simplifier;
        simplifier.setSampleRatio(LOD_list[i].reduction);
        LOD_model->accept(simplifier);

        osgDB::writeNodeFile(*LOD_model, LOD_path.toStdString());
    }

    LOD_count++;
    model_it++;

    ui->progressBar->setValue(LOD_count * 100 / used_models.count());
}
