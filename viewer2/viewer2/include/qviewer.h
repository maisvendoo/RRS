#ifndef     QVIEWER_H
#define     QVIEWER_H

#include    <QObject>

#include    <osgViewer/Viewer>
#include    <osgWidget/WindowManager>

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

    settings_t settings;

    osg::ref_ptr<osgWidget::WindowManager> wm;

    void initWindow(const settings_t &settings);

    void initMainMenu();

};

#endif // QVIEWER_H
