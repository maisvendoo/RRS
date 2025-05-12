#ifndef TRAFFIC_LIGHTS_HANDLER_H
#define TRAFFIC_LIGHTS_HANDLER_H

#include "settings.h"

#include <vsg/nodes/Group.h>

#include <QObject>

#include <string>

class TrafficLight;
class QByteArray;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler : public QObject
{
    Q_OBJECT

public:
    TrafficLightsHandler(const settings_t& settings, QObject* parent = nullptr);

    /// Get scene group
    vsg::ref_ptr<vsg::Group> getNode();

    void step(float t, float dt);

    bool load(QByteArray &data,
              const settings_t& settings,
              vsg::ref_ptr<vsg::Viewer> viewer,
              vsg::ref_ptr<vsg::Options> options);

private:

    void deserialize(QByteArray& data);
    void deserialize_signals(const char* signals_type, QDataStream& data_stream);

    void printSignalInfo(TrafficLight* tl);

    vsg::ref_ptr<vsg::Group> traffic_light_nodes = vsg::Group::create();

    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;

    std::string models_dir;
    std::string animations_dir;

public slots:
    void slotUpdateSignal(QByteArray data);
};

#endif // TRAFFIC_LIGHTS_HANDLER_H
