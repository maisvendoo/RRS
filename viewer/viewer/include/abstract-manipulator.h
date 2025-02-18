#ifndef ABSTRACT_MANIPULATOR_H
#define ABSTRACT_MANIPULATOR_H

#include "camera-position.h"

#include <osgGA/TrackballManipulator>

#include <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AbstractManipulator : public QObject, public osgGA::TrackballManipulator
{
    Q_OBJECT

public:
    AbstractManipulator(QObject* parent = Q_NULLPTR);

    bool handleFrame(const osgGA::GUIEventAdapter& event_adapter, osgGA::GUIActionAdapter& action_adapter);

protected:
    camera_position_t camera_position;

    double start_time;
    float delta_time;

    virtual ~AbstractManipulator();

    virtual void process_displays_lock();

signals:
    void lock_displays(bool lock);

public slots:
    void getCameraPosition(camera_position_t cp);
};

#endif // ABSTRACTMANIPULATOR_H
