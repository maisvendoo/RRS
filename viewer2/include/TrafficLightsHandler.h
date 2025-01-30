#ifndef TRAFFIC_LIGHTS_HANDLER_H
#define TRAFFIC_LIGHTS_HANDLER_H

#include "TrafficLight.h"
#include "settings.h"

#include <QByteArray>
#include <QMap>
#include <QString>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Group.h>

class TrafficLightsHandler
{
public:
    void deserialize(QByteArray& data);

    void create_pagedLODs(const settings_t& settings, vsg::ref_ptr<vsg::Options> options);

private:
    void printSignalInfo(TrafficLight* tl);

private:
    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;

    std::string models_dir;

    std::string animations_dir;
    vsg::ref_ptr<vsg::Group> traffic_light_nodes;
};

#endif // TRAFFIC_LIGHTS_HANDLER_H
