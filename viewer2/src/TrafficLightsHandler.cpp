#include "TrafficLightsHandler.h"
#include "ConfigReader.h"
#include "Logger.h"
#include "TrafficLight.h"
#include "filesystem.h"
#include <cstdint>
#include <qbuffer.h>
#include <qflags.h>
#include <qstringview.h>
#include <vsg/lighting/PointLight.h>
#include <vsg/lighting/SpotLight.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/utils/Builder.h>

TrafficLightsHandler::TrafficLightsHandler(QObject* parent)
    : QObject(parent)
{
}

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
}

void TrafficLightsHandler::loadSignalModels(const settings_t& settings, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings)
{
    traffic_light_nodes = vsg::Group::create();

    for (auto* tl : traffic_lights_fwd)
    {
        loadSignalModel(tl, settings, options, shadowSettings);
    }

    for (auto* tl :traffic_lights_bwd)
    {
        loadSignalModel(tl, settings, options, shadowSettings);
    }
}

void TrafficLightsHandler::printSignalInfo(TrafficLight* tl)
{
    LOG_INFO(
        "Signal %s at connector %s is initialized. Letter: %s | position: {%8.1f, %8.1f, %8.1f} | direction: %s {%6.3f %6.3f %6.3f}",
        tl->getModelName().toStdString().c_str(),
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

void TrafficLightsHandler::loadSignalModel(TrafficLight* tl, const settings_t& settings, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings)
{
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

    auto pagedLOD = vsg::PagedLOD::create();
    pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
    pagedLOD->filename = models_path + '/' + tl->getModelName().toStdString() + ".gltf";
    pagedLOD->options = options;

    int sd = tl->getSignalDirection();

    auto m1 = vsg::translate(tl->getPosition());

    vsg::vec3 o = tl->getOrth();
    vsg::vec3 r = tl->getRight();
    vsg::vec3 u = tl->getUp();

    vsg::mat4 m2(
         r.x, -o.x, u.x, 0,
        -r.y,  o.y, u.y, 0,
         r.z,  o.z, u.z, 0,
           0,    0,   0, 1
    );

    auto spotlight = vsg::SpotLight::create();
    spotlight->color.set(0.2f, 1.0f, 0.2f);
    spotlight->intensity = 1.0f;
    spotlight->position = vsg::vec3(0.0f, 0.0f, 5.0f);
    spotlight->direction = vsg::normalize(-o);
    spotlight->innerAngle = vsg::radians(15.0f);
    spotlight->outerAngle = vsg::radians(30.0f);
    spotlight->shadowSettings = shadowSettings;

    auto cullGroup = vsg::CullGroup::create();
    cullGroup->bound.radius = 200.0;
    cullGroup->addChild(spotlight);

    auto transform = vsg::MatrixTransform::create();
    transform->matrix = m2 * m1;

    tl->setNode(transform);
    tl->load_animations(animations_dir);

    transform->addChild(pagedLOD);
    transform->addChild(cullGroup);

    traffic_light_nodes->addChild(transform);
}
