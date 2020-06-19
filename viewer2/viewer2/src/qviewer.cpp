#include    "qviewer.h"

#include    "qt-events-handler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QViewer::QViewer(QObject *parent) : QObject(parent), osgViewer::Viewer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QViewer::~QViewer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int QViewer::run()
{
    osg::ref_ptr<QtEventsHandler> qt_events_hendler = new QtEventsHandler;
    this->addEventHandler(qt_events_hendler.get());

    return osgViewer::Viewer::run();
}
