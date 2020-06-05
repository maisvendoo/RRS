#include    "qthread-viewer.h"

#include    <QDir>
#include    <QApplication>

#include    "CfgReader.h"

#include    "esc-menu-handler.h"

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
    QString views_cfg = root_dir + QDir::separator() + "cfg" +
            QDir::separator() + "views.xml" ;

    if (!load_views_config(views_cfg))
    {
        return;
    }

    createMainMenu();

    this->moveToThread(vthread);
    connect(vthread, &QThread::started, this, &QThreadViewer::process);
    connect(vthread, &QThread::finished, this, &QThreadViewer::finish);
    vthread->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool QThreadViewer::load_common_settings(QString cfg_path)
{
    QString settings_path = root_dir + QDir::separator() + "cfg" +
            QDir::separator() + "settings.xml";

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool QThreadViewer::load_views_config(QString cfg_path)
{
    CfgReader cfg;

    if (!cfg.load(cfg_path))
    {
        return false;
    }

    QDomNode secNode = cfg.getFirstSection("View");

    while (!secNode.isNull())
    {
        camera_view_t cam_view;

        cfg.getInt(secNode, "x", cam_view.x);
        cfg.getInt(secNode, "y", cam_view.y);
        cfg.getInt(secNode, "width", cam_view.width);
        cfg.getInt(secNode, "height", cam_view.height);
        cfg.getBool(secNode, "fullscreen", cam_view.fullscreen);
        cfg.getBool(secNode, "show_title", cam_view.show_window_title);
        cfg.getInt(secNode, "screen_num", cam_view.screen_num);

        createView(cam_view);
        cam_view.view->addEventHandler(new EscMenuHandler);

        cviewer.addView(cam_view.view.get());

        camera_view.push_back(cam_view);

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::createView(camera_view_t &cv)
{
    cv.view = new osgViewer::View;

    if (cv.fullscreen)
    {
        cv.view->setUpViewOnSingleScreen(cv.screen_num);
    }
    else
    {
        cv.view->setUpViewInWindow(cv.x, cv.y, cv.width, cv.height);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::createCamera(camera_view_t &cv, settings_t &st)
{
    cv.camera = new osg::Camera;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::createMainMenu()
{
    cviewer.setKeyEventSetsDone(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::process()
{
    cviewer.run();

    vthread->quit();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QThreadViewer::finish()
{
    QApplication::quit();
}
