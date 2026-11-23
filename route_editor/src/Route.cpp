#include "editor/Route.h"

#include "editor/EditorContext.h"
#include "editor/Mask.h"
#include "editor/MergeSwitch.h"
#include "editor/RouteObject.h"
#include "editor/StaticObjectManager.h"
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

#include <vsg/app/CompileManager.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/app/Viewer.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/Value.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Path.h>
#include <vsg/io/read.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
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
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>
#include <vsg/utils/Builder.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

using PagedLodMap = std::map<std::string, vsg::ref_ptr<vsg::PagedLOD>>;

static vsg::ref_ptr<vsg::PagedLOD>
construct_paged_lod(const vsg::Path& filename, double radius,
                    const vsg::ref_ptr<vsg::Options>& options)
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
    : editor_ctx(context)
{
}

void Route::load()
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string objects_ref_path = fs.combinePath(route_dir,
                                                        "objects.ref");
    const std::string route_map_path = fs.combinePath(route_dir, "topology",
                                                      "map", "route1.map");

    if (!editor_ctx.objects_ref.read(objects_ref_path))
    {
        return;
    }

    if (!editor_ctx.route_map.read(route_map_path))
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

    const auto& camera_settings = editor_ctx.camera_settings;
    const auto& vsg_options = editor_ctx.vsg_options;
    auto& objects_ref = editor_ctx.objects_ref;
    auto& load_static_objects_thread = editor_ctx.load_static_objects_thread;
    auto& load_topology_thread = editor_ctx.load_topology_thread;

    for (auto& [label, rel_path] : objects_ref)
    {
        paged_lods.emplace(label, construct_paged_lod(
                                      fs.combinePath(route_dir, rel_path),
                                      camera_settings.view_distance,
                                      vsg_options));
    }

    load_static_objects_thread = std::thread(&Route::load_static_objects, this);
    load_topology_thread = std::thread(&Route::load_topology, this);
}

bool Route::load_stations_conf()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string stations_conf_path =
        fs.combinePath(route_dir, "topology", "stations.conf");

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
            editor_ctx.stations_conf[label] = translation;
        }
    }

    return true;
}

bool Route::load_waypoints_conf()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string waypoints_conf_path =
        fs.combinePath(route_dir, "topology", "waypoints.conf");

    std::ifstream waypoints_conf_file(waypoints_conf_path);
    if (!waypoints_conf_file)
    {
        Journal::instance()->error(
            QString("Failed to open %1").arg(waypoints_conf_path.c_str()));

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

            editor_ctx.waypoints_conf[label] = data;
        }
    }

    return true;
}

void Route::load_static_objects()
{
    for (const auto& [label, transforms] : editor_ctx.route_map)
    {
        const auto found_it = editor_ctx.objects_ref.find(label);
        if (found_it == editor_ctx.objects_ref.cend())
        {
            continue;
        }

        const std::string& rel_path = found_it->second;
        const auto& paged_lod = paged_lods[label];

        for (const auto& transform : transforms)
        {
            const vsg::dvec3& pos = transform.pos;
            const vsg::dvec3& rot_deg = transform.rot_deg;

            const auto object = RouteObject::create(editor_ctx, paged_lod,
                                                    label, pos, -rot_deg);

            auto& viewer = editor_ctx.viewer;
            vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
            auto compile_result = viewer->compileManager->compile(object);

            viewer->addUpdateOperation(MergeSwitch::create(observer_viewer,
                                                     vsg::ref_ptr(this),
                                                     vsg::MASK_ALL, object,
                                                     compile_result));

            editor_ctx.static_objects.lock()->emplace_back(object);

            // auto& object_manager = editor_ctx.object_manager;
            // object_manager->add_object(label, rel_path, paged_lod, pos, rot_deg,
            //     vsg::dvec3{1.0, 1.0, 1.0});
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
        Journal::instance()->error(
            QString("Failed to find field \"SignalModelsDir\" "
                    "in section %1 in %2")
                .arg(section_name)
                .arg(cfg_path.c_str()));

        return false;
    }

    const std::string models_dir_name = signal_models_dir.toStdString();

    const std::string models_dir = fs.combinePath(fs.getDataDir(), "models",
                                                  models_dir_name);

    Journal::instance()->info(QString("Signals directory %1")
                                  .arg(models_dir.c_str()));

    const signals_data_t* signals_data = nullptr;

    auto topology_guard = editor_ctx.topology.lock();
    auto& topology = *topology_guard;

    topology = std::make_unique<Topology>();

    const auto directory_name = std::filesystem::path(route_dir).filename();

    editor_ctx.finish_topology_thread.store(false);
    if (!topology->load(directory_name.string().c_str(), true,
                        &editor_ctx.finish_topology_thread))
    {
        Journal::instance()->error("Failed to load topology");
        return false;
    }
    editor_ctx.topology_loaded.store(true);

    signals_data = topology->getSignalsData();
    if (!signals_data)
    {
        return false;
    }

    PagedLodMap paged_lods;

    const auto& camera_settings = editor_ctx.camera_settings;
    const auto& vsg_options = editor_ctx.vsg_options;

    const auto load_signals = [&](const std::vector<Signal*>& signals_) -> void
    {
        for (Signal* const signal : signals_)
        {
            if (!signal)
            {
                Journal::instance()->error(
                    QString("Invalid signal %1")
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

            const std::string signal_model_path =
                fs.combinePath(models_dir, signal_model_name) + ".gltf";

            vsg::ref_ptr<vsg::PagedLOD> paged_lod;

            auto paged_lod_it = paged_lods.find(signal_model_path);
            if (paged_lod_it == paged_lods.end())
            {
                const auto new_paged_lod = construct_paged_lod(
                    signal_model_path, camera_settings.view_distance,
                    vsg_options);

                paged_lod_it = paged_lods.emplace(signal_model_path,
                                                  new_paged_lod).first;
            }

            paged_lod = paged_lod_it->second;

            const vsg::dvec3 pos = signal->getPos();
            const vsg::dvec3 right = signal->getRight();
            const vsg::dvec3 orth = signal->getOrth();
            const vsg::dvec3 up = signal->getUp();

            const vsg::dvec3 rotation_deg = {
                vsg::degrees(std::atan2(orth.z, up.z)),
                vsg::degrees(std::atan2(-right.z, std::hypot(orth.z, up.z))),
                vsg::degrees(std::atan2(-right.y, right.x))
            };

            const auto object = RouteObject::create(editor_ctx, paged_lod,
                                                    signal_model_name, pos,
                                                    -rotation_deg);

            auto& viewer = editor_ctx.viewer;
            vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
            auto compile_result = viewer->compileManager->compile(object);

            viewer->addUpdateOperation(MergeSwitch::create(observer_viewer,
                                                           vsg::ref_ptr(this),
                                                           vsg::MASK_ALL,
                                                           object,
                                                           compile_result));

            editor_ctx.static_objects.lock()->emplace_back(object);
        }
    };

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    const auto group = vsg::Group::create();

    const std::string shaders_dir_path = fs.combinePath(fs.getDataDir(),
                                                        "shaders");

    const auto input_assembly_state = vsg::InputAssemblyState::create();
    input_assembly_state->topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;
    rasterization_state->lineWidth = 2.0f;

    const auto state_group = create_state_group_with_custom_pipeline(
        shaders_dir_path.c_str(), "traj_line.vert", "traj_line.frag",
        vsg_options,
        vsg::VertexInputState::Bindings{
            VkVertexInputBindingDescription{0, sizeof(vsg::vec3),
                                            VK_VERTEX_INPUT_RATE_VERTEX},
            VkVertexInputBindingDescription{1, sizeof(vsg::vec3),
                                            VK_VERTEX_INPUT_RATE_VERTEX}
        },
        vsg::VertexInputState::Attributes{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT,
                                              0},
            VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT,
                                              0}
        },
        vsg::DescriptorSetLayoutBindings{}, vsg::Descriptors{},
        input_assembly_state, rasterization_state,
        vsg::MultisampleState::create(), vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create());

    srand(time(NULL));

    auto font_path = fs.combinePath(fs.getFontsDir(),
                                    "JetBrainsMono-Regular.ttf");
    auto font = vsg::read_cast<vsg::Font>(font_path, vsg_options);
    auto text_shader_set = vsg_options->shaderSets["text"] =
        vsg::createTextShaderSet(vsg_options);

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
            points.emplace_back(track.begin_point);
        }

        points.emplace_back(tracks.back().end_point);

        const auto vertices = vsg::vec3Array::create(points_size * 2);
        const auto colors = vsg::vec3Array::create(points_size * 2);
        const auto indices = vsg::ushortArray::create(points_size * 4 - 2);

        vsg::vec3 color;
        color.r = (float)rand() / (float)RAND_MAX;
        color.g = (float)rand() / (float)RAND_MAX;
        color.b = (float)rand() / (float)RAND_MAX;

        vsg::dvec3 pos = {0.0, 0.0, 0.0};
        std::size_t vertex_index = 0;
        std::size_t color_index = 0;
        std::size_t index_index = 0;
        for (std::size_t i = 0; i < points_size; ++i)
        {
            vertices->at(vertex_index++) = points[i];
            vertices->at(vertex_index++) = points[i] +
                                           vsg::dvec3(0.0, 0.0, 10.0);
            colors->at(color_index++) = color;
            colors->at(color_index++) = color;
            indices->at(index_index++) = i * 2;
            indices->at(index_index++) = i * 2 + 1;
            if (i > 0)
            {
                indices->at(index_index++) = i * 2;
                indices->at(index_index++) = (i - 1) * 2;
            }
            pos += points[i];
        }
        pos /= points_size;

        const auto geometry = vsg::Geometry::create();
        geometry->assignArrays(vsg::DataList{vertices, colors});
        geometry->assignIndices(indices);
        geometry->commands.push_back(
            vsg::DrawIndexed::create(indices->size(), 1, 0, 0, 0));

        state_group->addChild(geometry);

        auto layout = vsg::StandardLayout::create();
        layout->billboardAutoScaleDistance = true;
        layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->verticalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->position = pos;
        layout->position += {0.0, 0.0,
                             (float)rand() / (float)RAND_MAX * 4.0f + 1.0f};
        layout->horizontal = vsg::vec3(3.0, 0.0, 0.0);
        layout->vertical = vsg::vec3(0.0, 3.0, 0.0);
        layout->color = {color, 1.0f};
        layout->billboard = true;
        auto text = vsg::Text::create();
        text->font = font;
        text->layout = layout;
        text->text = vsg::stringValue::create(trajectory->getName()
                                                  .toStdString().c_str());
        text->setup(0, vsg_options);
        group->addChild(text);
    }

    group->addChild(state_group);

    auto& viewer = editor_ctx.viewer;
    vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
    auto compile_result = viewer->compileManager->compile(group);

    viewer->addUpdateOperation(MergeSwitch::create(observer_viewer,
                                                   vsg::ref_ptr(this),
                                                   vsg::Mask{MASK_GUI2}, group,
                                                   compile_result));

    return true;
}
