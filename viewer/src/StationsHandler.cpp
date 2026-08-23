#include "StationsHandler.h"

#include "Logger.h"
#include "filesystem.h"

#include <QDataStream>
#include <QIODevice>

#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/text/Font.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

namespace
{
    /// Радиус сферы отсечения станции, м
    constexpr double STATION_CULLING_RADIUS = 50.0;

    /// Высота текста имени станции, м
    constexpr float STATION_NAME_SIZE = 50.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StationsHandler::load(QByteArray& stations_data, vsg::ref_ptr<vsg::Options> options)
{
    deserialize(stations_data);

    LOG_INFO("Loaded %lld stations", static_cast<long long>(stations.size()));

    if (stations.empty())
    {
        LOG_WARN("Stations list is empty");
        return false;
    }

    createSceneGraph(options);

    return root != nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationsHandler::deserialize(QByteArray& data)
{
    // Очищаем список станций
    stations.clear();

    QDataStream stream(&data, QIODevice::ReadOnly);

    uint32_t stations_count = 0;
    stream >> stations_count;

    for (uint32_t i = 0; i < stations_count; ++i)
    {
        QByteArray station_data;
        stream >> station_data;

        topology_station_t station;
        station.deserialize(station_data);
        stations.push_back(station);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationsHandler::createSceneGraph(vsg::ref_ptr<vsg::Options> options)
{
    FileSystem& fs = FileSystem::getInstance();
    const std::string font_path = fs.combinePath(fs.getFontsDir(), "JetBrainsMono-Regular.ttf");

    auto font = vsg::read_cast<vsg::Font>(font_path, options);
    if (!font)
    {
        LOG_ERROR("Fail to load station names font: %s", font_path.c_str());
        return;
    }

    auto group = vsg::Group::create();

    for (const auto& station : stations)
    {

        auto layout = vsg::StandardLayout::create();
        layout->position = vsg::vec3(static_cast<float>(station.pos_x),
                                     static_cast<float>(station.pos_y),
                                     static_cast<float>(station.pos_z + 10.0));
        layout->horizontal = vsg::vec3(STATION_NAME_SIZE, 0.0f, 0.0f);
        layout->vertical = vsg::vec3(0.0f, STATION_NAME_SIZE, 0.0f);
        layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->billboard = true;

        auto text = vsg::Text::create();
        text->text = vsg::wstringValue::create(station.name.toStdWString());
        text->font = font;
        text->layout = layout;
        text->setup(0, options);

        auto cull_node = vsg::CullNode::create();
        cull_node->bound = vsg::dsphere(station.pos_x, station.pos_y, station.pos_z, STATION_CULLING_RADIUS);
        cull_node->child = text;

        group->addChild(cull_node);
    }

    root = vsg::Switch::create();
    root->addChild(vsg::MASK_ALL, group);

    LOG_INFO("Created scene graph for %lld stations",
             static_cast<long long>(group->children.size()));
}
