#pragma once
#ifndef TRAFFIC_LIGHTS_HANDLER_H
#define TRAFFIC_LIGHTS_HANDLER_H

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <QMap>
#include <QObject>
#include <QString>

#include <string>

class WorldCulling;
class TrafficLight;
class QByteArray;

namespace vsg
{
    class Options;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler final : public QObject
{
    Q_OBJECT

public:
    explicit TrafficLightsHandler(QObject* parent = nullptr);

    void step(float t, float dt);

    bool load(QByteArray& data, const std::string& route_dir_full_path,
              vsg::ref_ptr<WorldCulling> world_culling, vsg::ref_ptr<vsg::Options> options);

private:
    void deserialize(QByteArray& data);
    void deserialize_signals(const char* signals_type, QDataStream& data_stream);

    void printSignalInfo(const TrafficLight* tl) const;

    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;

    std::string models_dir;
    std::string animations_dir;

public slots:
    void slotUpdateSignal(QByteArray data);
};

#endif // TRAFFIC_LIGHTS_HANDLER_H
