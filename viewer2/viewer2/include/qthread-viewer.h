#ifndef     QTHREAD_VIEWER_H
#define     QTHREAD_VIEWER_H

#include    <QObject>
#include    <QThread>

#include    <osgViewer/CompositeViewer>

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

    osgViewer::CompositeViewer  cviewer;

private slots:

    void process();
};

#endif // QTHREAD_VIEWER_H
