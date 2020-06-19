#ifndef     QVIEWER_H
#define     QVIEWER_H

#include    <QObject>

#include    <osgViewer/Viewer>

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

private:


};

#endif // QVIEWER_H
