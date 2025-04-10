#include "TrafficLightsHandler.h"

#include "CfgReader.h"
#include "TrafficLight.h"
#include "filesystem.h"
#include "Logger.h"
#include "MyGui.h"
#include "settings.h"

#include <iostream>
#include <qcontainerfwd.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/read.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>

#include <QBuffer>
#include <QDir>
#include <QDirIterator>

#include <cstdint>
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/utils/SharedObjects.h>

TrafficLightsHandler::TrafficLightsHandler(QObject* parent, vsg::ref_ptr<vsg::Options> options)
    : QObject(parent)
    , options(options)
{
}

void TrafficLightsHandler::deserialize(QByteArray& data)
{
    // Очищаем список сигналов
    traffic_lights_fwd.clear();
    traffic_lights_bwd.clear();

    loaded = false;
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);

    deserialize_signals("Line", stream);
    deserialize_signals("Enter", stream);
    deserialize_signals("Exit", stream);
}

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

void TrafficLightsHandler::create_pagedLODs(const settings_t& settings)
{
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

    std::string models_path = fs.getDataDir();
    models_path = fs.combinePath(models_path, "models");
    models_path = fs.combinePath(models_path, models_dir);

    QDir models(QString(models_path.c_str()));
    QDirIterator models_files(models.path(), QStringList() << "*.gltf", QDir::NoDotAndDotDot | QDir::Files);

    while (models_files.hasNext())
    {
        QString fullPath = models_files.next();
        QFileInfo fileInfo(fullPath);

        QString model_base_name = fileInfo.baseName();

        signal_nodes_paths.insert(model_base_name, fullPath);
    }
}

void TrafficLightsHandler::loadSignalModels(const settings_t& settings, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings)
{
    LOG_INFO("Start loading signal models");
    traffic_light_nodes = vsg::Group::create();

    for (auto* traffic_light : traffic_lights_fwd)
    {
        loadSignalModel(traffic_light, settings, shadowSettings);
    }

    for (auto* traffic_light :traffic_lights_bwd)
    {
        loadSignalModel(traffic_light, settings, shadowSettings);
    }
    LOG_INFO("Finished loading signal models");

    loaded = true;
}

void TrafficLightsHandler::step(float t, float dt)
{
    if (!loaded)
    {
        return;
    }

    for (auto* traffic_light : traffic_lights_fwd)
    {
        traffic_light->step(t, dt);
    }

    for (auto* traffic_light : traffic_lights_bwd)
    {
        traffic_light->step(t, dt);
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
        (tl->getSignalDirection() == -1) ? "BWD" : "FWD",
        tl->getOrth().x,
        tl->getOrth().y,
        tl->getOrth().z
    );
}

void TrafficLightsHandler::loadSignalModel(TrafficLight* traffic_light, const settings_t& settings, vsg::ref_ptr<vsg::ShadowSettings> shadowSettings)
{
    static std::map<std::string, vsg::ref_ptr<vsg::Node>> loaded_nodes;

    std::string node_path = signal_nodes_paths.value(traffic_light->getModelName(), "").toStdString();
    if (node_path.empty())
    {
        return;
    }

    vsg::dmat4 m1 = vsg::translate(traffic_light->getPosition());

    vsg::dvec3 o(traffic_light->getOrth());
    vsg::dvec3 r(traffic_light->getRight());
    vsg::dvec3 u(traffic_light->getUp());

    vsg::dmat4 m2(
        r.x,  -o.x,  u.x,  0,
        -r.y,   o.y,  u.y,  0,
        r.z,   o.z,  u.z,  0,
        0,     0,    0,  1
    );

    auto global_transform = vsg::MatrixTransform::create();
    global_transform->matrix = m1 * m2;

    vsg::ref_ptr<vsg::Node> signal_node;
    if (loaded_nodes.count(node_path))
    {
        signal_node = loaded_nodes[node_path];
    }
    else
    {
        signal_node = vsg::read_cast<vsg::Node>(node_path, options);
        loaded_nodes.emplace(node_path, signal_node);
        if (!signal_node)
        {
            LOG_INFO("Failed to load model from %s", node_path.c_str());
            return;
        }

        if (auto cull_node = vsg::ref_ptr(signal_node->cast<vsg::CullNode>()))
        {
            if (auto outer_transform = vsg::ref_ptr(cull_node->child->cast<vsg::MatrixTransform>()))
            {
                if (auto old_outer_group = vsg::ref_ptr(outer_transform->children[0]->cast<vsg::Group>()))
                {
                    auto new_outer_group = vsg::Group::create();

                    for (auto& child : old_outer_group->children)
                    {
                        std::string name;
                        child->getValue("name", name);

                        auto transform = vsg::MatrixTransform::create();
                        transform->setValue("name", name);
                        transform->addChild(child);
                        new_outer_group->addChild(transform);
                    }

                    outer_transform->children[0] = new_outer_group;
                }
            }
        }
    }

    global_transform->addChild(signal_node);

    auto pdo = vsg::PropagateDynamicObjects::create();
    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    traffic_light->set_node(global_transform);
    traffic_light->load_animations(animations_dir, options, pdo, duplicate);

    // global_transform->addChild(signal_node);
    global_transform->traverse(*pdo);

    if (!pdo->dynamicObjects.empty())
    {
        for (auto& object : pdo->dynamicObjects)
        {
            if (!duplicate->contains(object))
            {
                duplicate->insert(object);
            }
        }

        signal_node = copyop(signal_node);
    }

    global_transform->children = {signal_node};

    // GUIParams::nodes.emplace_back(global_transform);

    traffic_light_nodes->addChild(global_transform);

    // auto pagedLOD = vsg::PagedLOD::create();
    // pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
    // pagedLOD->filename = node_path.toStdString();
    // pagedLOD->options = options;

    // traffic_light_nodes->addChild(pagedLOD);

    // int sd = tl->getSignalDirection();

    // auto spotlight = vsg::SpotLight::create();
    // spotlight->color.set(0.2f, 1.0f, 0.2f);
    // spotlight->intensity = 1.0f;
    // spotlight->position = vsg::vec3(0.0f, 0.0f, 5.0f);
    // spotlight->direction = vsg::normalize(-o);
    // spotlight->innerAngle = vsg::radians(15.0f);
    // spotlight->outerAngle = vsg::radians(30.0f);
    // spotlight->shadowSettings = shadowSettings;

    // auto cullGroup = vsg::CullGroup::create();
    // cullGroup->bound.radius = 200.0;
    // cullGroup->addChild(spotlight);

    // transform->addChild(cullGroup);
}

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
