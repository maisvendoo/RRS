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

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:

    camera_position_t cp;

    double  start_time;
    float   delta_time;

    virtual ~AbstractManipulator();

    virtual void keysDownProcess(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa);

    virtual void keysUpProcess(const osgGA::GUIEventAdapter &ea,
                               osgGA::GUIActionAdapter &aa);

    virtual void dragMouseProcess(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa);

    virtual void moveMouseProcess(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa);

    virtual void pushMouseProcess(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa);

    virtual void releaseMouseProcess(const osgGA::GUIEventAdapter &ea,
                                     osgGA::GUIActionAdapter &aa);

    virtual void scrollProcess(const osgGA::GUIEventAdapter &ea,
                               osgGA::GUIActionAdapter &aa);

    virtual void update(osg::Camera *camera);

public slots:

    void getCameraPosition(camera_position_t cp);
};

#endif // ABSTRACTMANIPULATOR_H
