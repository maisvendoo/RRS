#include "Route.h"

#include "EditorContext.h"
#include "Journal.h"
#include "Mask.h"
#include "ObjectManager.h"
#include "PagedLodMap.h"
#include "RouteMap.h"
#include "RouteObject.h"
#include "filesystem.h"
#include "graphics/pipeline_funcs.h"
#include "rail-signal.h"
#include "signals-data-types.h"
#include "topology.h"
#include "trajectory.h"
#include "vec3.h"
#include "settings/CameraSettings.h"

#include <CfgReader.h>

#include <vsg/app/RecordTraversal.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/read.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/Builder.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

Route::Route(
    EditorContext& context,
    const camera_settings_t& camera_settings,
    const vsg::ref_ptr<vsg::Options>& vsg_options,
    const std::string& route_dir,
    const vsg::ref_ptr<Gizmo>& gizmo,
    ObjectManager& object_manager
)
    : context_(context)
    , camera_settings(camera_settings)
    , vsg_options(vsg_options)
    , route_dir(route_dir)
    , gizmo(gizmo)
    , object_manager(object_manager)
{
    const bool success = load_objects_ref() && load_route_map()
        && load_stations_conf() && load_waypoints_conf();

    if (!success)
    {
        return;
    }

    const FileSystem& fs = FileSystem::getInstance();

    for (auto& [label, ref] : context.objects_ref)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(route_dir,
            ref.relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            camera_settings.view_distance);

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = vsg_options;

        ref.paged_lod = paged_lod;
    }

    context.load_static_objects_thread = std::thread(
        &Route::load_static_objects, this);

    context.load_topology_thread = std::thread(
        &Route::load_topology, this);
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(
        route_dir, "objects.ref");

    std::ifstream objects_ref_file(objects_ref_path);
    if (!objects_ref_file)
    {
        Journal::instance()->error(QString("Failed to open %1")
            .arg(objects_ref_path.c_str()));

        return false;
    }

    std::string line;
    while (std::getline(objects_ref_file, line))
    {
        std::istringstream iss(std::move(line));
        std::string label, relative_path;

        if (iss >> label >> relative_path)
        {
            context_.objects_ref.emplace(std::move(label),
                ObjectRef{std::move(relative_path), nullptr});
        }
    }

    return true;
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_map_path = fs.combinePath(route_dir,
        "topology", "map", "route1.map");

    std::ifstream route_map_file(route_map_path);
    if (!route_map_file)
    {
        Journal::instance()->error(QString("Failed to open %1")
            .arg(route_map_path.c_str()));

        return false;
    }

    std::string line;
    while (std::getline(route_map_file, line))
    {
        if (line.empty())
        {
            continue;
        }

        if (line.back() == ';')
        {
            line.pop_back();
        }

        std::replace(line.begin(), line.end(), ',', ' ');

        std::istringstream iss(std::move(line));
        std::string label;
        vsg::dvec3 translation, rotation;

        if (iss >> label >> translation >> rotation)
        {
            context_.route_map[label].emplace_back(
                RouteMapTransformation{translation, rotation});
        }
    }

    std::size_t total_static_objects_count = 0;
    for (const auto& [label, transforms] : context_.route_map)
    {
        total_static_objects_count += transforms.size();
    }
    context_.total_static_objects_count = total_static_objects_count;

    return true;
}

bool Route::load_stations_conf()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string stations_conf_path = fs.combinePath(route_dir,
        "topology", "stations.conf");

    std::ifstream stations_conf_file(stations_conf_path);
    if (!stations_conf_file)
    {
        Journal::instance()->error(QString("Failed to open %1")
            .arg(stations_conf_path.c_str()));

        return false;
    }

    std::string line;
    while (std::getline(stations_conf_file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream iss(std::move(line));
        std::string label;
        vsg::dvec3 translation;
        if (iss >> label >> translation)
        {
            context_.stations_conf[label] = translation;
        }
    }

    return true;
}

bool Route::load_waypoints_conf()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string waypoints_conf_path = fs.combinePath(route_dir,
        "topology", "waypoints.conf");

    std::ifstream waypoints_conf_file(waypoints_conf_path);
    if (!waypoints_conf_file)
    {
        Journal::instance()->error(QString("Failed to open %1")
            .arg(waypoints_conf_path.c_str()));

        return false;
    }

    std::string line;
    while (std::getline(waypoints_conf_file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream iss(std::move(line));
        std::string label;
        WaypointData data;
        std::string direction_string;
        std::string coord_string;
        std::string length_string;

        if (std::getline(iss, label, '\t') &&
            std::getline(iss, data.trajectory_name, '\t') &&
            std::getline(iss, direction_string, '\t') &&
            std::getline(iss, coord_string, '\t') &&
            std::getline(iss, length_string, '\t'))
        {
            data.direction = stoi(direction_string);
            data.coord = stod(coord_string);
            data.length = stod(length_string);

            context_.waypoints_conf[label] = data;
        }
    }

    return true;
}

void Route::load_static_objects()
{
    for (const auto& [label, transforms] : context_.route_map)
    {
        const auto ref_it = context_.objects_ref.find(label);
        if (ref_it == context_.objects_ref.cend())
        {
            continue;
        }

        for (const auto& transform : transforms)
        {
            const auto object = RouteObject::create(context_,
                ref_it->second.paged_lod, gizmo, label, transform.translation,
                -transform.rotation_deg);

            context_.compile_infos.emplace_back(CompileInfo{
                vsg::ref_ptr(this), object, vsg::MASK_ALL});

            std::lock_guard<std::mutex> lock_guard(context_.static_objects_mutex);
            context_.static_objects.emplace_back(object);
            ++context_.static_objects_count;
        }
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string cfg_path = fs.combinePath(route_dir,
        "topology", "models-config.xml");

    CfgReader cfg;
    if (!cfg.load(QString::fromStdString(cfg_path)))
    {
        Journal::instance()->error(QString("Failed to load %1")
            .arg(cfg_path.c_str()));

        return false;
    }

    const QString section_name = "Models";
    QString signal_models_dir;

    if (!cfg.getString(section_name, "SignalModelsDir", signal_models_dir))
    {
        Journal::instance()->error(QString("Failed to find field "
            "\"SignalModelsDir\" in section %1 in %2")
            .arg(section_name)
            .arg(cfg_path.c_str()));

        return false;
    }

    const std::string models_dir_name = signal_models_dir.toStdString();

    const std::string models_dir = fs.combinePath(fs.getDataDir(),
        "models", models_dir_name);

    Journal::instance()->info(QString("Signals directory %1").arg(models_dir.c_str()));

    context_.topology_mutex.lock();
    context_.topology = std::make_unique<Topology>();
    context_.topology_mutex.unlock();

    const auto directory_name = std::filesystem::path(
        route_dir).filename();

    if (!context_.topology->load(directory_name.string().c_str()))
    {
        Journal::instance()->error("Failed to load topology");
        return false;
    }
    context_.topology_loaded = true;

    PagedLodMap paged_lods;

    const signals_data_t* const signals_data = context_.topology->getSignalsData();
    if (!signals_data)
    {
        return false;
    }

    const auto load_signals = [&](const std::vector<Signal*>& signals_) -> void
    {
        for (Signal* const signal : signals_)
        {
            if (!signal)
            {
                Journal::instance()->error(QString("Invalid signal %1")
                    .arg(reinterpret_cast<quintptr>(signal)));

                continue;
            }

            const std::string signal_model_name =
                signal->getSignalModel().toStdString();

            if (signal_model_name.empty() || signal_model_name == "empty_line")
            {
                ++context_.topology_objects_count;
                continue;
            }

            const std::string signal_model_path = fs.combinePath(
                models_dir, signal_model_name) + ".gltf";

            vsg::ref_ptr<vsg::PagedLOD> paged_lod;

            auto paged_lod_it = paged_lods.find(signal_model_path);
            if (paged_lod_it == paged_lods.end())
            {
                const auto new_paged_lod = vsg::PagedLOD::create();
                new_paged_lod->filename = signal_model_path;

                new_paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
                    camera_settings.view_distance);

                new_paged_lod->children.front() = {0.1, nullptr};
                new_paged_lod->options = vsg_options;

                paged_lod_it = paged_lods.emplace(signal_model_path,
                    new_paged_lod).first;
            }

            paged_lod = paged_lod_it->second;

            const vsg::dvec3 pos = to_vsg_vec3(signal->getPos());
            const vsg::dvec3 right = to_vsg_vec3(signal->getRight());
            const vsg::dvec3 orth = to_vsg_vec3(signal->getOrth());
            const vsg::dvec3 up = to_vsg_vec3(signal->getUp());

            const vsg::dvec3 rotation_deg = {
                vsg::degrees(atan2(orth.z, up.z)),
                vsg::degrees(atan2(-right.z, hypot(orth.z, up.z))),
                vsg::degrees(atan2(-right.y, right.x))
            };

            const auto object = RouteObject::create(context_, paged_lod, gizmo,
                signal_model_name, pos, -rotation_deg);

            context_.compile_infos.emplace_back(CompileInfo{
                vsg::ref_ptr(this), object, vsg::MASK_ALL});

            ++context_.topology_objects_count;
        }
    };

    context_.total_topology_objects_count += signals_data->line_signals.size();
    context_.total_topology_objects_count += signals_data->enter_signals.size();
    context_.total_topology_objects_count += signals_data->exit_signals.size();

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    const auto group = vsg::Group::create();

    const std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    const auto input_assembly_state = vsg::InputAssemblyState::create();
    input_assembly_state->topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;

    const auto state_group = create_state_group_with_custom_pipeline(
        shaders_dir_path.c_str(),
        "traj_line.vert",
        "traj_line.frag",
        vsg_options,
        vsg::VertexInputState::Bindings{
            VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
            VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}
        },
        vsg::VertexInputState::Attributes{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}
        },
        vsg::DescriptorSetLayoutBindings{},
        vsg::Descriptors{},
        input_assembly_state,
        rasterization_state,
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()
    );

    const traj_list_t* traj_list = context_.topology->getTrajectoriesList();
    for (const Trajectory* trajectory : *traj_list)
    {
        const auto& tracks = trajectory->getTracks();
        if (tracks.empty())
        {
            continue;
        }

        const std::size_t tracks_size = tracks.size();
        const std::size_t points_size = tracks_size + 1;

        std::vector<vsg::dvec3> points;
        points.reserve(points_size);

        for (const track_t& track : tracks)
        {
            const dvec3& p = track.begin_point;
            points.emplace_back(vsg::dvec3{p.x, p.y, p.z});
        }

        const dvec3& p = tracks.back().end_point;
        points.emplace_back(vsg::dvec3{p.x, p.y, p.z});

        const auto vertices = vsg::vec3Array::create(points_size);
        const auto colors = vsg::vec3Array::create(points_size);
        const auto indices = vsg::ushortArray::create(points_size);

        for (std::size_t i = 0; i < points_size; ++i)
        {
            vertices->at(i) = points[i];
            colors->at(i) = {1.0f, 1.0f, 0.0f};
            indices->at(i) = i;
        }

        const auto geometry = vsg::Geometry::create();
        geometry->assignArrays(vsg::DataList{vertices, colors});
        geometry->assignIndices(indices);
        geometry->commands.push_back(vsg::DrawIndexed::create(
            points_size, 1, 0, 0, 0
        ));

        state_group->addChild(geometry);
    }

    group->addChild(state_group);

    context_.compile_infos.emplace_back(CompileInfo{vsg::ref_ptr(this),
        group, vsg::Mask{MASK_GUI2}});

    return true;
}
