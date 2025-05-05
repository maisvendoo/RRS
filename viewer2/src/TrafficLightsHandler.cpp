#include "TrafficLightsHandler.h"
#include "TrafficLight.h"

#include "CfgReader.h"
#include "filesystem.h"
#include "Logger.h"
// #include "MyGui.h"

#include <vsg/app/Viewer.h>

#include <QBuffer>

#include <cstdint>

TrafficLightsHandler::TrafficLightsHandler(const settings_t &settings, QObject* parent)
    : QObject(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Group> TrafficLightsHandler::getNode()
{
    return traffic_light_nodes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::step(float t, float dt)
{
    for (auto* traffic_light : traffic_lights_fwd)
    {
        traffic_light->step(t, dt);
    }

    for (auto* traffic_light : traffic_lights_bwd)
    {
        traffic_light->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrafficLightsHandler::load(QByteArray &data, const settings_t &settings, vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Options> options)
{
    deserialize(data);

    FileSystem& fs =FileSystem::getInstance();
    std::string path = fs.combinePath(settings.route_dir_full_path, "topology");
    path = fs.combinePath(path, "models-config.xml");

    QString tmp_qstr = path.c_str();
    CfgReader cfg;
    if (cfg.load(tmp_qstr))
    {
        QString sec_name = "Models";

        tmp_qstr = "";
        if (cfg.getString(sec_name, "SignalModelsDir", tmp_qstr))
            models_dir = tmp_qstr.toStdString();

        tmp_qstr = "";
        if (cfg.getString(sec_name, "SignalAnimationsDir", tmp_qstr))
            animations_dir = tmp_qstr.toStdString();
    }

    std::string models_dir_path = fs.getDataDir();
    models_dir_path = fs.combinePath(models_dir_path, "models");
    models_dir_path = fs.combinePath(models_dir_path, models_dir);

    LOG_INFO("Signals directory: %s ", models_dir_path.c_str());
    LOG_INFO("Start adding signal models");

    for (auto* traffic_light : traffic_lights_fwd)
    {
        traffic_light->loadSignal(models_dir_path,
                                  animations_dir,
                                  viewer,
                                  options);
        traffic_light_nodes->addChild(traffic_light->transform);
    }

    for (auto* traffic_light : traffic_lights_bwd)
    {
        traffic_light->loadSignal(models_dir_path,
                                  animations_dir,
                                  viewer,
                                  options);
        traffic_light_nodes->addChild(traffic_light->transform);
    }
    LOG_INFO("Finished adding signal models");

    //GUIParams::nodes.emplace_back(traffic_light_nodes);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::deserialize(QByteArray& data)
{
    // Очищаем список сигналов
    traffic_lights_fwd.clear();
    traffic_lights_bwd.clear();

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);

    deserialize_signals("Line", stream);
    deserialize_signals("Enter", stream);
    deserialize_signals("Exit", stream);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::deserialize_signals(const char* signals_type, QDataStream& data_stream)
{
    std::uint32_t signal_count;
    data_stream >> signal_count;
    LOG_INFO("%s signals: %u", signals_type, signal_count);

    for (std::uint32_t i = 0; i < signal_count; ++i)
    {
        QByteArray data;
        data_stream >> data;

        TrafficLight* traffic_light = new TrafficLight;
        traffic_light->deserialize(data);

        if (traffic_light->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(traffic_light);

        if (traffic_light->getSignalDirection() == -1)
        {
            traffic_lights_bwd.insert(traffic_light->getConnectorName(), traffic_light);
        }
        else
        {
            traffic_lights_fwd.insert(traffic_light->getConnectorName(), traffic_light);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::printSignalInfo(TrafficLight* tl)
{
    LOG_INFO(
        "Signal %s at connector %s is initialized. Letter: %s | position: {%8.1f, %8.1f, %8.1f} | direction: %s {%6.3f %6.3f %6.3f}",
        tl->getModelName().toStdString().c_str(),
        tl->getConnectorName().toStdString().c_str(),
        tl->getLetter().toStdString().c_str(),
        tl->position.x,
        tl->position.y,
        tl->position.z,
        (tl->getSignalDirection() == -1) ? "BWD" : "FWD",
        tl->orth.x,
        tl->orth.y,
        tl->orth.z
    );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::slotUpdateSignal(QByteArray data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    QString connector_name = "";
    int signal_dir = 0;

    stream >> connector_name;
    stream >> signal_dir;

    if (connector_name.isEmpty())
    {
        return;
    }

    TrafficLight* traffic_light = (signal_dir == -1) ?
        traffic_lights_bwd.value(connector_name, nullptr) :
        traffic_lights_fwd.value(connector_name, nullptr);

    if (traffic_light)
    {
        traffic_light->deserialize(data);
    }
}
