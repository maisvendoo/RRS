#include    "qthread-viewer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QThreadViewer::QThreadViewer(QString root_dir,
                             QString route_path,
                             QString train_path,
                             QObject *parent) : QObject(parent)
  , vthread(new QThread)
  , root_dir(root_dir)
  , route_path(route_path)
  , train_path(train_path)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QThreadViewer::~QThreadViewer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::start()
{
    osg::ref_ptr<osgViewer::View> v0 = new osgViewer::View;
    v0->setUpViewInWindow(0, 0, 1024, 768);
    cviewer.addView(v0.get());

    this->moveToThread(vthread);
    connect(vthread, &QThread::started, this, &QThreadViewer::process);
    vthread->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::process()
{
    cviewer.run();
}
