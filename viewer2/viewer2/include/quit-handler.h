#ifndef     QUIT_HANDLER_H
#define     QUIT_HANDLER_H

#include    <QObject>

#include    <osgGA/GUIEventHandler>
#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QuitHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    QuitHandler(QObject *parent = Q_NULLPTR)
        : QObject(parent)
        , osgGA::GUIEventHandler()
        , done(false)
    {

    }

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa)
    {
        if ( (ea.getEventType() == osgGA::GUIEventAdapter::FRAME))
        {
            osgViewer::Viewer *v = static_cast<osgViewer::Viewer *>(&aa);
            v->setDone(done);
        }

        return false;
    }

public slots:

    void quit()
    {
        done = true;
    }

private:

    bool done;
};

#endif // QUITHANDLER_H
