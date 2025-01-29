#ifndef TRAIN_LIGHTS_HANDLER_H
#define TRAIN_LIGHTS_HANDLER_H

#include "TrafficLight.h"

#include <QByteArray>
#include <QMap>
#include <QString>

class TrainLightsHandler
{
public:
    void deserialize(QByteArray& data);

private:
    void printSignalInfo(TrafficLight* tl);

private:
    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;
};

#endif // TRAIN_LIGHTS_HANDLER_H
