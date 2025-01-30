#include "TrafficLightsHandler.h"
#include "ConfigReader.h"
#include "Logger.h"
#include "TrafficLight.h"
#include "filesystem.h"
#include <cstdint>
#include <filesystem>
#include <qbuffer.h>
#include <qflags.h>
#include <qstringview.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/PagedLOD.h>

// TODO: remove duplication
void TrafficLightsHandler::deserialize(QByteArray& data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    std::uint32_t line_signals_size = 0;
    stream >> line_signals_size;

    LOG_INFO("Line signals: %u", line_signals_size);

    // Очищаем список сигналов
    traffic_lights_fwd.clear();
    traffic_lights_bwd.clear();

    for (std::uint32_t i = 0; i < line_signals_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight* tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        if (tl->getSignalDirection() == -1)
        {
            traffic_lights_bwd.insert(tl->getConnectorName(), tl);
        }
        else
        {
            traffic_lights_fwd.insert(tl->getConnectorName(), tl);
        }
    }

    std::uint32_t enter_signals_size = 0;
    stream >> enter_signals_size;

    LOG_INFO("Enter signals: %u", enter_signals_size);

    for (std::uint32_t i = 0; i < enter_signals_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight* tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        if (tl->getSignalDirection() == -1)
        {
            traffic_lights_bwd.insert(tl->getConnectorName(), tl);
        }
        else
        {
            traffic_lights_fwd.insert(tl->getConnectorName(), tl);
        }
    }

    std::uint32_t exit_signals_size = 0;
    stream >> exit_signals_size;

    LOG_INFO("Exit signals: %u", exit_signals_size);

    for (std::uint32_t i = 0; i < exit_signals_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight* tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        if (tl->getSignalDirection() == -1)
        {
            traffic_lights_bwd.insert(tl->getConnectorName(), tl);
        }
        else
        {
            traffic_lights_fwd.insert(tl->getConnectorName(), tl);
        }
    }
}

void TrafficLightsHandler::create_pagedLODs(const settings_t& settings, vsg::ref_ptr<vsg::Options> options)
{
    traffic_light_nodes = vsg::Group::create();

    FileSystem& fs = FileSystem::getInstance();

    std::string path = fs.combinePath(settings.route_dir_full_path, "topology");
    path = fs.combinePath(path, "models-config.xml");

    ConfigReader cfg_reader(path);
    cfg_reader.setSection("Models");
    cfg_reader.getValue("SignalModelsDir", models_dir);
    cfg_reader.getValue("SignalAnimationsDir", animations_dir);

    std::string models_path = fs.getDataDir();
    models_path = fs.combinePath(models_path, "models");
    models_path = fs.combinePath(models_path, models_dir);

    std::filesystem::directory_iterator dir_it(models_path);
    for (auto& entry : dir_it)
    {
        if (entry.path().extension() == ".gltf")
        {
            std::string fullPath = entry.path().string();

            auto pagedLOD = vsg::PagedLOD::create();
            pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
            pagedLOD->filename = fullPath;
            pagedLOD->options = options;
        }
    }
}

void TrafficLightsHandler::printSignalInfo(TrafficLight* tl)
{
    /*
    QString msg = QString("Signal at connector %1 is initialized. Letter: %2 | position: {%3, %4, %5} | direction: %6 {%7, %8, %9}")
                      .arg(tl->getConnectorName())
                      .arg(tl->getLetter())
                      .arg(tl->getPosition().x(), 8, 'f', 1)
                      .arg(tl->getPosition().y(), 8, 'f', 1)
                      .arg(tl->getPosition().z(), 8, 'f', 1)
                      .arg(tl->getSignalDirection() == -1 ? "BWD" : "FWD")
                      .arg(tl->getOrth().x(), 6, 'f', 3)
                      .arg(tl->getOrth().y(), 6, 'f', 3)
                      .arg(tl->getOrth().z(), 6, 'f', 3);*/

    LOG_INFO(
        "Signal at connector %s is initialized. Letter: %s | position: {%8.1f, %8.1f, %8.1f} | direction: %s {%6.3f %6.3f %6.3f}",
        tl->getConnectorName().toStdString().c_str(),
        tl->getLetter().toStdString().c_str(),
        tl->getPosition().x,
        tl->getPosition().y,
        tl->getPosition().z,
        (tl->getSignalDirection() == -1) ? "BWD" : "FDW",
        tl->getOrth().x,
        tl->getOrth().y,
        tl->getOrth().z
    );
}
