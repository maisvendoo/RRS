#ifndef     TRAFFIC_LIGHTS_HANDLER_H
#define     TRAFFIC_LIGHTS_HANDLER_H

#include    <QObject>
#include    <QMap>

#include    <osgGA/GUIEventHandler>

#include    <traffic-light.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    TrafficLightsHandler(QObject *parent = Q_NULLPTR);

    ~TrafficLightsHandler();

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    void deserialize(QByteArray &data);

private:

    QMap<QString, TrafficLight *> traffic_lights;
};

#endif
