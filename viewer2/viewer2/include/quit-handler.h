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

    QuitHandler(QObject *parent = Q_NULLPTR);


    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);


public slots:

    void quit();

private:

    bool done;
};

#endif // QUIT_HANDLER_H
