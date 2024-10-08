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
