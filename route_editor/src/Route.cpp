#include "editor/Route.h"

#include "editor/EditorContext.h"
#include "editor/Mask.h"
#include "editor/ObjectManager.h"
#include "editor/PagedLodMap.h"
#include "editor/RouteMap.h"
#include "editor/RouteObject.h"
#include "editor/settings/CameraSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <filesystem.h>
#include <graphics/pipeline_funcs.h>
#include <rail-signal.h>
#include <signals-data-types.h>
#include <topology.h>
#include <trajectory.h>
#include <vec3.h>

#include <vsg/app/RecordTraversal.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Path.h>
#include <vsg/io/read.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
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
#include <memory>
#include <sstream>
#include <string>
#include <thread>

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

static vsg::ref_ptr<vsg::PagedLOD> construct_paged_lod(const vsg::Path& filename,
    double radius, const vsg::ref_ptr<vsg::Options>& options)
{
    const auto paged_lod = vsg::PagedLOD::create();
    paged_lod->filename = filename;
    paged_lod->bound.set(vsg::dvec3(0.0, 0.0, 0.0), radius);
    paged_lod->children[0].minimumScreenHeightRatio = 0.1;
    paged_lod->children[0].node = nullptr;
    paged_lod->options = options;
    return paged_lod;
}

Route::Route(EditorContext& context)
    : editor_context(context)
{
}

void Route::load()
{
    if (!load_objects_ref())
    {
        return;
    }

    if (!load_route_map())
    {
        return;
    }

    if (!load_stations_conf())
    {
        return;
    }

    if (!load_waypoints_conf())
    {
        return;
    }

    const auto& fs = FileSystem::getInstance();
    const auto& camera_settings = editor_context.camera_settings;
    const auto& vsg_options = editor_context.vsg_options;
    auto& objects_ref = editor_context.objects_ref;
    auto& load_static_objects_thread = editor_context.load_static_objects_thread;
    auto& load_topology_thread = editor_context.load_topology_thread;

    for (auto& [label, ref] : objects_ref)
    {
        ref.paged_lod = construct_paged_lod(
            fs.combinePath(route_dir, ref.relative_path),
            camera_settings.view_distance, vsg_options);
    }

    load_static_objects_thread = std::thread(&Route::load_static_objects, this);
    load_topology_thread = std::thread(&Route::load_topology, this);
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
        std::istringstream iss(line);
        std::string label, relative_path;

        if (iss >> label >> relative_path)
        {
            editor_context.objects_ref.emplace(std::move(label),
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

        std::istringstream iss(line);
        std::string label;
        vsg::dvec3 translation, rotation;

        if (iss >> label >> translation >> rotation)
        {
            editor_context.route_map[label].emplace_back(
                RouteMapTransformation{translation, rotation});
        }
    }

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

        std::istringstream iss(line);
        std::string label;
        vsg::dvec3 translation;
        if (iss >> label >> translation)
        {
            editor_context.stations_conf[label] = translation;
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

        std::istringstream iss(line);
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

            editor_context.waypoints_conf[label] = data;
        }
    }

    return true;
}

void Route::load_static_objects()
{
    for (const auto& [label, transforms] : editor_context.route_map)
    {
        const auto ref_it = editor_context.objects_ref.find(label);
        if (ref_it == editor_context.objects_ref.cend())
        {
            continue;
        }

        for (const auto& transform : transforms)
        {
            const auto object = RouteObject::create(editor_context,
                ref_it->second.paged_lod, label, transform.translation,
                -transform.rotation_deg);

            editor_context.compile_infos.lock()->emplace_back(CompileInfo{
                vsg::ref_ptr(this), object, vsg::MASK_ALL});

            editor_context.static_objects.lock()->emplace_back(object);

            constexpr vsg::dvec3 X_AXIS = {1.0, 0.0, 0.0};
            constexpr vsg::dvec3 Y_AXIS = {0.0, 1.0, 0.0};
            constexpr vsg::dvec3 Z_AXIS = {0.0, 0.0, 1.0};

            auto matrix_transform = vsg::MatrixTransform::create();
            matrix_transform->children = {ref_it->second.paged_lod};
            matrix_transform->matrix = vsg::translate(transform.translation) *
                vsg::rotate(vsg::radians(transform.rotation_deg.z), Z_AXIS) *
                vsg::rotate(vsg::radians(transform.rotation_deg.y), Y_AXIS) *
                vsg::rotate(vsg::radians(transform.rotation_deg.x), X_AXIS);

            auto& object_manager = editor_context.object_manager;

            object_manager->labels.push_back(label);
            object_manager->relative_paths.push_back(ref_it->second.relative_path);
            object_manager->matrix_transforms.push_back(matrix_transform);
            object_manager->initial_matrixes.push_back(matrix_transform->matrix);
            object_manager->is_selected.push_back(false);
        }
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string cfg_path = fs.combinePath(route_dir, "topology",
        "models-config.xml");

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

    const signals_data_t* signals_data = nullptr;

    auto topology_guard = editor_context.topology.lock();
    auto& topology = *topology_guard;

    topology = std::make_unique<Topology>();

    const auto directory_name = std::filesystem::path(route_dir).filename();

    editor_context.finish_topology_thread.store(false);
    if (!topology->load(directory_name.string().c_str(), true,
        &editor_context.finish_topology_thread))
    {
        Journal::instance()->error("Failed to load topology");
        return false;
    }
    editor_context.topology_loaded.store(true);

    signals_data = topology->getSignalsData();
    if (!signals_data)
    {
        return false;
    }

    PagedLodMap paged_lods;

    const auto& camera_settings = editor_context.camera_settings;
    const auto& vsg_options = editor_context.vsg_options;

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
            signal->calcPosition();

            const std::string signal_model_name =
                signal->getSignalModel().toStdString();

            if (signal_model_name.empty() || signal_model_name == "empty_line")
            {
                continue;
            }

            const std::string signal_model_path = fs.combinePath(
                models_dir, signal_model_name) + ".gltf";

            vsg::ref_ptr<vsg::PagedLOD> paged_lod;

            auto paged_lod_it = paged_lods.find(signal_model_path);
            if (paged_lod_it == paged_lods.end())
            {
                const auto new_paged_lod = construct_paged_lod(signal_model_path,
                    camera_settings.view_distance, vsg_options);

                paged_lod_it = paged_lods.emplace(signal_model_path, new_paged_lod).first;
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

            const auto object = RouteObject::create(editor_context, paged_lod,
                signal_model_name, pos, -rotation_deg);

            editor_context.compile_infos.lock()->emplace_back(CompileInfo{
                vsg::ref_ptr(this), object, vsg::MASK_ALL});

            editor_context.static_objects.lock()->emplace_back(object);
        }
    };

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    const auto group = vsg::Group::create();

    const std::string shaders_dir_path = fs.combinePath(fs.getDataDir(), "shaders");

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

    const traj_list_t* traj_list = topology->getTrajectoriesList();
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

    editor_context.compile_infos.lock()->emplace_back(CompileInfo{
        vsg::ref_ptr(this), group, vsg::Mask{MASK_GUI2}});

    return true;
}
