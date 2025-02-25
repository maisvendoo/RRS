#ifndef TRAFFIC_LIGHTS_HANDLER_H
#define TRAFFIC_LIGHTS_HANDLER_H

#include "TrafficLight.h"
#include "settings.h"

#include <QByteArray>
#include <QMap>
#include <QString>
#include <qassert.h>
#include <qcontainerfwd.h>
#include <qmap.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/nodes/Group.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler : public QObject
{
    Q_OBJECT

public:
    TrafficLightsHandler(QObject* parent = Q_NULLPTR);

    void deserialize(QByteArray& data);

    void create_pagedLODs(const settings_t& settings, vsg::ref_ptr<vsg::Options> options);

    void loadSignalModels(const settings_t& settings, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings);

    vsg::ref_ptr<vsg::Group> traffic_light_nodes;

    void step(float t, float dt);

signals:

    void updateViewer();

private:
    void printSignalInfo(TrafficLight* tl);

    void loadSignalModel(TrafficLight* tl, const settings_t& settings, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings);

private:

    QMap<QString, TrafficLight*> traffic_lights_fwd;
    QMap<QString, TrafficLight*> traffic_lights_bwd;

    QMap<QString, QString> signal_nodes_paths;

    std::string models_dir;

    std::string animations_dir;

    std::set<std::string> handled_paths;

public slots:

    void slotUpdateSignal(QByteArray data);
};

#endif // TRAFFIC_LIGHTS_HANDLER_H
