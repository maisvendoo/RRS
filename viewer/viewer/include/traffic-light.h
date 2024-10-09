#ifndef     TRAFFIC_LIGHT_H
#define     TRAFFIC_LIGHT_H

#include    <QObject>
#include    <signal-types.h>
#include    <osg/Group>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLight : public QObject
{
    Q_OBJECT

public:

    TrafficLight(QObject *parent = Q_NULLPTR);

    ~TrafficLight();

    void deserialize(QByteArray &data);

    QString getConnectorName() const
    {
        return conn_name;
    }

    QString getLetter() const
    {
        return letter;
    }

    osg::Vec3d getPosition() const
    {
        return pos;
    }

    QString getModelName() const
    {
        return signal_model;
    }

    osg::Vec3d getOrth() const
    {
        return orth;
    }

    osg::Vec3d getRight() const
    {
        return right;
    }

    osg::Vec3d getUp() const
    {
        return up;
    }

    int getSignalDirection() const
    {
        return signal_dir;
    }

private:

    lens_state_t lens_state;

    QString letter = "";

    QString signal_model = "";

    int signal_dir = 0;

    QString conn_name = "";

    bool is_busy = false;

    /// Орт вдоль оси X собственной системы координат светофора
    osg::Vec3d right;

    /// Орт вдоль оси Y собственной системы координат светофора
    osg::Vec3d orth;

    /// Орт вдоль оси Z собственной системы координат светофора
    osg::Vec3d up;

    /// Абсолютное положение сигнала
    osg::Vec3d pos;
};

#endif
