#ifndef     QVIEWER_H
#define     QVIEWER_H

#include    <QObject>

#include    <osgViewer/Viewer>

#include    "settings.h"
#include    "command-line.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QViewer : public QObject, public osgViewer::Viewer
{
    Q_OBJECT

public:

    QViewer(QObject *parent = Q_NULLPTR);

    ~QViewer();

    int run();

    void init(const settings_t &settings,
              const command_line_t &cmd_line);

private:

    osg::ref_ptr<osg::Camera>   camera;

    void initWindow(const settings_t &settings);
};

#endif // QVIEWER_H
