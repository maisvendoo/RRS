#include "Route.h"

#include "EditorContext.h"
#include "Mask.h"
#include "PagedLodMap.h"
#include "RouteMap.h"
#include "RouteObject.h"
#include "Settings.h"
#include "filesystem.h"
// #include "parse_file_funcs.h"
#include "rail-signal.h"
#include "signals-data-types.h"
#include "topology-defines.h"
#include "topology.h"
#include "trajectory.h"
#include "vec3.h"

#include <CfgReader.h>

#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vsg/app/RecordTraversal.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Geometry.h>

#include <QString>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/RasterizationState.h>
#include <vulkan/vulkan_core.h>

#define LABEL_BUFFER_SIZE 256
#define RELATIVE_PATH_BUFFER_SIZE 512
#define FLOAT_BUFFER_SIZE 32

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

Route::Route(EditorContext& context)
    : context_(context)
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
        paged_lod->filename = fs.combinePath(context.route_dir,
            ref.relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            context.settings.view_distance);

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = context.options;

        ref.paged_lod = paged_lod;
    }

    load_static_objects();
    load_topology();
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(
        context_.route_dir, "objects.ref");

    // char label[LABEL_BUFFER_SIZE];
    // char relative_path[RELATIVE_PATH_BUFFER_SIZE];

    // return parse_file_line_by_line(
    //     objects_ref_path.c_str(), "r", " \t\r",
    //     [&]() -> void {
    //         context.objects_ref.emplace(label, relative_path);
    //     },
    //     {
    //         ParseField::String(label, LABEL_BUFFER_SIZE),
    //         ParseField::String(relative_path, RELATIVE_PATH_BUFFER_SIZE)
    //     }
    // );

    std::ifstream objects_ref_file(objects_ref_path);
    if (!objects_ref_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", objects_ref_path.c_str());
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

    const std::string route_map_path = fs.combinePath(context_.route_dir,
        "topology", "map", "route1.map");

    // char label[LABEL_BUFFER_SIZE];
    // char float_buffer[FLOAT_BUFFER_SIZE];
    // vsg::vec3 translation;
    // vsg::vec3 rotation_deg;

    // return parse_file_line_by_line(
    //     route_map_path.c_str(), "r", " \t\r,;",
    //     [&]() -> void {
    //         context.route_map[label].emplace_back(
    //             RouteMapTransformation{translation, rotation_deg});
    //     },
    //     {
    //         ParseField::String(label, LABEL_BUFFER_SIZE),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.x),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.y),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.z),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.x),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.y),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.z)
    //     }
    // );

    std::ifstream route_map_file(route_map_path);
    if (!route_map_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", route_map_path.c_str());
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

    return true;
}

bool Route::load_stations_conf()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string stations_conf_path = fs.combinePath(context_.route_dir,
        "topology", "stations.conf");

    std::ifstream stations_conf_file(stations_conf_path);
    if (!stations_conf_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", stations_conf_path.c_str());
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

    const std::string waypoints_conf_path = fs.combinePath(context_.route_dir,
        "topology", "waypoints.conf");

    std::ifstream waypoints_conf_file(waypoints_conf_path);
    if (!waypoints_conf_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", waypoints_conf_path.c_str());
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
                ref_it->second.paged_lod, label, transform.translation,
                -transform.rotation_deg);

            this->addChild(vsg::MASK_ALL, object);

            std::lock_guard<std::mutex> lock_guard(context_.static_objects_mutex);
            context_.static_objects.emplace_back(object);
        }
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string cfg_path = fs.combinePath(context_.route_dir,
        "topology", "models-config.xml");

    CfgReader cfg;
    if (!cfg.load(QString::fromStdString(cfg_path)))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to load %s\n", cfg_path.c_str());
        return false;
    }

    const QString section_name = "Models";
    QString signal_models_dir;

    if (!cfg.getString(section_name, "SignalModelsDir", signal_models_dir))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to find field \"SignalModelsDir\" "
            "in section %s in %s\n", section_name.toStdString().c_str(),
            cfg_path.c_str());

        return false;
    }

    const std::string models_dir_name = signal_models_dir.toStdString();

    const std::string models_dir = fs.combinePath(fs.getDataDir(),
        "models", models_dir_name);

    // TODO: Replace on Journal
    std::printf("Signals directory: %s\n", models_dir.c_str());

    context_.topology_mutex.lock();
    context_.topology = std::make_unique<Topology>();

    const auto directory_name = std::filesystem::path(
        context_.route_dir).filename();

    if (!context_.topology->load(directory_name.string().c_str()))
    {
        context_.topology_mutex.unlock();
        // TODO: Replace on Journal
        std::fputs("Failed to load topology\n", stderr);
        return false;
    }
    context_.topology_mutex.unlock();

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
                // TODO: Replace on Journal
                std::fprintf(stderr, "Invalid signal %p\n", (void*)signal);
                return;
            }

            const std::string signal_model_name =
                signal->getSignalModel().toStdString();

            if (signal_model_name.empty() || signal_model_name == "empty_line")
            {
                return;
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
                    context_.settings.view_distance);

                new_paged_lod->children.front() = {0.1, nullptr};
                new_paged_lod->options = context_.options;

                paged_lod_it = paged_lods.emplace(signal_model_path,
                    new_paged_lod).first;
            }

            paged_lod = paged_lod_it->second;

            signal->calcPosition();

            const vsg::dvec3 pos = to_vsg_vec3(signal->getPos());
            const vsg::dvec3 right = to_vsg_vec3(signal->getRight());
            const vsg::dvec3 orth = to_vsg_vec3(signal->getOrth());
            const vsg::dvec3 up = to_vsg_vec3(signal->getUp());

            const vsg::dvec3 rotation_deg = {
                vsg::degrees(atan2(orth.z, up.z)),
                vsg::degrees(atan2(-right.z, hypot(orth.z, up.z))),
                vsg::degrees(atan2(-right.y, right.x))
            };

            const auto object = RouteObject::create(context_, paged_lod,
                signal_model_name, pos, rotation_deg);

            this->addChild(vsg::MASK_ALL, object);
        }
    };

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    // const auto input_assembly_state = vsg::InputAssemblyState::create();
    // input_assembly_state->topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    // const auto rasterization_state = vsg::RasterizationState::create();
    // rasterization_state->lineWidth = 15.0f;
    // rasterization_state->cullMode = VK_CULL_MODE_NONE;
    // rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;

    // const auto state_group = vsg::StateGroup::create();
    // state_group->add(input_assembly_state);
    // state_group->add(rasterization_state);

    // const traj_list_t* traj_list = context_.topology->getTrajectoriesList();
    // for (const Trajectory* trajectory : *traj_list)
    // {
    //     const auto& tracks = trajectory->getTracks();
    //     const std::size_t tracks_size = tracks.size();

    //     if (tracks_size < 2)
    //     {
    //         continue;
    //     }

    //     std::vector<vsg::dvec3> points;
    //     points.reserve(tracks_size);
    //     for (const track_t& track : tracks)
    //     {
    //         points.emplace_back(vsg::dvec3{track.begin_point.x,
    //             track.begin_point.y, track.begin_point.z});
    //     }

    //     const auto vertices = vsg::vec3Array::create(tracks_size);
    //     const auto colors = vsg::vec4Array::create(tracks_size);
    //     const auto indices = vsg::ushortArray::create(tracks_size);

    //     for (std::size_t i = 0; i < tracks_size; ++i)
    //     {
    //         vertices->at(i) = points[i];
    //         colors->at(i).set(1.0f, 1.0f, 0.0f, 1.0f);
    //         indices->at(i) = i;
    //     }

    //     const auto geometry = vsg::Geometry::create();
    //     geometry->assignArrays(vsg::DataList{vertices});
    //     geometry->assignIndices(indices);
    //     geometry->commands.push_back(vsg::DrawIndexed::create(
    //         tracks_size, 1, 0, 0, 0
    //     ));

    //     state_group->addChild(geometry);
    // }

    // context_.compile_infos.emplace_back(CompileInfo{context_.route,
    //     state_group, vsg::Mask{MASK_GUI2}});

    return true;
}
