#include    "quit-handler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QuitHandler::QuitHandler(QObject *parent)
    : QObject(parent)
    , osgGA::GUIEventHandler()
    , done(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool QuitHandler::handle(const osgGA::GUIEventAdapter &ea,
                    osgGA::GUIActionAdapter &aa)
{
    if ( (ea.getEventType() == osgGA::GUIEventAdapter::FRAME))
    {
        osgViewer::Viewer *v = static_cast<osgViewer::Viewer *>(&aa);
        v->setDone(done);
    }

    return false;
}

void QuitHandler::quit()
{
    done = true;
}
