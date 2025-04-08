#ifndef TRAFFIC_LIGHTS_HANDLER_H
#define TRAFFIC_LIGHTS_HANDLER_H

#include "settings.h"
#include "TrafficLight.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/nodes/Group.h>

#include <QByteArray>
#include <QObject>

#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler : public QObject
{
    Q_OBJECT

public:
    TrafficLightsHandler(QObject* parent = Q_NULLPTR, vsg::ref_ptr<vsg::Options> options = {});

    void deserialize(QByteArray& data);

    void create_pagedLODs(const settings_t& settings);

    void loadSignalModels(const settings_t& settings, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings);

    vsg::ref_ptr<vsg::Group> traffic_light_nodes;

    void step(float t, float dt);

private:
    void printSignalInfo(TrafficLight* tl);

    void loadSignalModel(TrafficLight* tl, const settings_t& settings, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings);

private:
    vsg::ref_ptr<vsg::Options> options;

    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;

    QMap<QString, QString> signal_nodes_paths;

    std::string models_dir;
    std::string animations_dir;

    bool loaded = false;

public slots:
    void slotUpdateSignal(QByteArray data);
};

#endif // TRAFFIC_LIGHTS_HANDLER_H
