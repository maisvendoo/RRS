#ifndef     ABSTRACT_MANIPULATOR_H
#define     ABSTRACT_MANIPULATOR_H

#include    <QObject>

#include    <osgGA/TrackballManipulator>

#include    "camera-position.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AbstractManipulator : public QObject, public osgGA::TrackballManipulator
{
    Q_OBJECT

public:

    AbstractManipulator(QObject *parent = Q_NULLPTR);    

    bool handleFrame(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:

    camera_position_t cp;

    double  start_time;
    float   delta_time;

    virtual ~AbstractManipulator();    

public slots:

    void getCameraPosition(camera_position_t cp);
};

#endif // ABSTRACTMANIPULATOR_H
