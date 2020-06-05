#ifndef     QTHREAD_VIEWER_H
#define     QTHREAD_VIEWER_H

#include    <QObject>
#include    <QThread>

#include    <osgViewer/CompositeViewer>

#include    "camera-view.h"
#include    "settings.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QThreadViewer : public QObject
{
    Q_OBJECT

public:

    QThreadViewer(QString root_dir,
                  QString route_path,
                  QString train_path,
                  QObject *parent = Q_NULLPTR);

    ~QThreadViewer();

    void start();

private:

    QThread                     *vthread;

    QString                     root_dir;

    QString                     route_path;

    QString                     train_path;

    settings_t                  settings;

    std::vector<camera_view_t>  camera_view;

    osgViewer::CompositeViewer  cviewer;

    bool load_common_settings(QString cfg_path);

    bool load_views_config(QString cfg_path);

    void createView(camera_view_t &cv);

    void createCamera(camera_view_t &cv, settings_t &st);

    void createMainMenu();

private slots:

    void process();

    void finish();
};

#endif // QTHREAD_VIEWER_H
