#include "editor/Route.h"

#include "editor/EditorContext.h"
#include "editor/Mask.h"
#include "editor/RouteObject.h"
#include "editor/TrackFurniture.h"
#include "editor/TrackProfile.h"
#include "editor/settings/CameraSettings.h"

#include <Journal.h>
#include <filesystem.h>
#include <graphics/pipeline_funcs.h>
#include <rail-signal.h>
#include <signals-data-types.h>
#include <topology.h>
#include <topology-defines.h>
#include <track.h>
#include <trajectory.h>
#include <vec3.h>

#include <CfgReader.h>

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
#include <vsg/nodes/Group.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

/// Сборка state group с шейдерами линий траекторий (traj_line.vert/frag)
vsg::ref_ptr<vsg::StateGroup> create_trajectory_lines_state_group(
    vsg::ref_ptr<const vsg::Options> options);

Route::Route(EditorContext& context)
    : context_(context)
{
    load_geo_anchor();

    const bool success = load_objects_ref() && load_route_map()
        && load_stations_conf() && load_waypoints_conf();

    if (!success)
    {
        return;
    }

    // Выбор траектории и подсветка предыдущего маршрута не действуют
    // в новом: сбрасываем до запуска фоновых потоков
    context_.selected_trajectory = nullptr;
    context_.selected_trajectory_name.clear();
    context_.trajectory_highlight_switch = vsg::Switch::create();
    context_.build_preview_switch = vsg::Switch::create();

    // Профили участков пути (track-edit.conf, отсутствие файла - не ошибка)
    load_track_profiles();

    // Группа сгенерированного обвеса пути (окно «Путь» -> «Генерация»):
    // опоры КС, платформы, километровые столбики; сама группа пуста
    // до генерации/восстановления из конфига
    context_.generated_group = vsg::Group::create();
    context_.generated_items.clear();

    this->addChild(vsg::MASK_ALL, context_.generated_group);

    const FileSystem& fs = FileSystem::getInstance();

    // Создаём PagedLOD для каждой модели objects.ref
    // (подгрузка с диска выполняется DatabasePager в фоне)
    for (auto& [label, ref] : context.objects_ref)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(context.route_dir,
            ref.relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            context_.camera_settings.view_distance);

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = context.options;

        ref.paged_lod = paged_lod;
    }

    context.load_static_objects_thread = std::thread(
        &Route::load_static_objects, this);

    context.load_topology_thread = std::thread(
        &Route::load_topology, this);
}

//------------------------------------------------------------------------------
/// Гео-якорь маршрута: Latitude/Longitude из description.xml (центр
/// области импорта; для импортированных OSM/GPX - реальный)
//------------------------------------------------------------------------------
void Route::load_geo_anchor()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string path = fs.combinePath(context_.route_dir,
                                            "description.xml");

    CfgReader cfg;

    if (!cfg.load(QString::fromStdString(path)))
    {
        return;
    }

    cfg.getDouble("Route", "Latitude", context_.route_latitude);
    cfg.getDouble("Route", "Longitude", context_.route_longitude);
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(
        context_.route_dir, "objects.ref");

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

    const std::string route_map_path = fs.combinePath(context_.route_dir,
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

    const std::string stations_conf_path = fs.combinePath(context_.route_dir,
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

    const std::string waypoints_conf_path = fs.combinePath(context_.route_dir,
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
            data.direction = std::stoi(direction_string);
            data.coord = std::stod(coord_string);
            data.length = std::stod(length_string);

            context_.waypoints_conf[label] = data;
        }
    }

    return true;
}

bool Route::load_track_profiles()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string track_edit_path = fs.combinePath(context_.route_dir,
        "track-edit.conf");

    // Глобальная функция из TrackProfile.h (не путать с методом):
    // профили + отложенный обвес + слои объектов
    return ::load_track_edit_config(track_edit_path, context_);
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

            // Слой объекта из track-edit.conf (секция Layer): запись
            // опознаётся по метке и позиции
            for (const LayerConfig& layer_config : context_.layer_configs)
            {
                if (layer_config.object_label == label &&
                    vsg::length(transform.translation -
                        layer_config.position) < 0.01)
                {
                    object->layer = layer_config.name;
                    break;
                }
            }

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

    const std::string cfg_path = fs.combinePath(context_.route_dir,
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
        context_.route_dir).filename();

    if (!context_.topology->load(directory_name.string().c_str()))
    {
        Journal::instance()->error("Failed to load topology");
        return false;
    }
    context_.topology_loaded = true;

    // Сигналы: модели по models-config.xml (кэш PagedLOD по пути модели)
    std::map<std::string, vsg::ref_ptr<vsg::PagedLOD>> paged_lods;

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
                    context_.camera_settings.view_distance);

                new_paged_lod->children.front() = {0.1, nullptr};
                new_paged_lod->options = context_.options;

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

            const auto object = RouteObject::create(context_, paged_lod,
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

    // Линии траекторий
    const auto state_group =
        create_trajectory_lines_state_group(context_.options);

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
            indices->at(i) = static_cast<unsigned short>(i);
        }

        const auto geometry = vsg::Geometry::create();
        geometry->assignArrays(vsg::DataList{vertices, colors});
        geometry->assignIndices(indices);
        geometry->commands.push_back(vsg::DrawIndexed::create(
            points_size, 1, 0, 0, 0
        ));

        state_group->addChild(geometry);
    }

    const auto group = vsg::Group::create();
    group->addChild(state_group);

    // Подсветка выбранной траектории (режим "Пути", клавиша P):
    // вторая геометрия цианом поверх жёлтых линий, включается
    // по требованию в update_trajectory_highlight
    if (context_.trajectory_highlight_switch)
    {
        const auto highlight_state_group =
            create_trajectory_lines_state_group(context_.options);

        highlight_state_group->addChild(context_.trajectory_highlight_switch);
        highlight_state_group->addChild(context_.build_preview_switch);

        group->addChild(highlight_state_group);
    }

    context_.compile_infos.emplace_back(CompileInfo{
        vsg::ref_ptr(this), group, vsg::MASK_ALL});

    // Восстановление сгенерированного обвеса пути из track-edit.conf:
    // построение требует сэмплирования траекторий, поэтому выполняется
    // только сейчас - после загрузки топологии
    TrackFurniture::restore_all(context_);

    return true;
}

void Route::select_trajectory(const std::string& name)
{
    Trajectory* trajectory = nullptr;

    {
        std::lock_guard<std::mutex> lock_guard(context_.topology_mutex);

        if (context_.topology && !name.empty())
        {
            const traj_list_t* const traj_list =
                context_.topology->getTrajectoriesList();

            const auto traj_it = traj_list->find(QString::fromStdString(name));

            if (traj_it != traj_list->cend())
            {
                trajectory = traj_it.value();
            }
        }
    }

    if (trajectory != nullptr)
    {
        context_.selected_trajectory = trajectory;
        context_.selected_trajectory_name = name;
    }
    else
    {
        context_.selected_trajectory = nullptr;
        context_.selected_trajectory_name.clear();
    }

    update_trajectory_highlight(trajectory);
}

void Route::update_trajectory_highlight(const Trajectory* trajectory)
{
    const auto highlight_switch = context_.trajectory_highlight_switch;

    if (!highlight_switch)
    {
        return;
    }

    // Убираем предыдущую подсветку (по образцу DeleteObjects)
    if (!highlight_switch->children.empty())
    {
        highlight_switch->children.clear();

        context_.compile_infos.emplace_back(CompileInfo{
            nullptr, vsg::ref_ptr(this)});
    }

    if (trajectory == nullptr)
    {
        return;
    }

    const auto& tracks = trajectory->getTracks();

    if (tracks.empty())
    {
        return;
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
        // Чуть приподнимаем подсветку над жёлтой линией,
        // чтобы не конфликтовать с ней по глубине
        vertices->at(i) = points[i] + vsg::dvec3{0.0, 0.0, 0.3};

        // Цвет циан: выделенная траектория подсвечивается им
        colors->at(i) = {0.0f, 1.0f, 1.0f};
        indices->at(i) = static_cast<unsigned short>(i);
    }

    const auto geometry = vsg::Geometry::create();
    geometry->assignArrays(vsg::DataList{vertices, colors});
    geometry->assignIndices(indices);
    geometry->commands.push_back(vsg::DrawIndexed::create(
        points_size, 1, 0, 0, 0
    ));

    highlight_switch->addChild(vsg::MASK_ALL, geometry);

    context_.compile_infos.emplace_back(CompileInfo{
        highlight_switch, geometry, vsg::MASK_ALL});
}

void Route::show_build_preview(const Trajectory* trajectory,
                                double begin_m, double end_m)
{
    const auto preview_switch = context_.build_preview_switch;

    if (!preview_switch)
    {
        return;
    }

    // Убираем предыдущее превью
    if (!preview_switch->children.empty())
    {
        preview_switch->children.clear();

        context_.compile_infos.emplace_back(CompileInfo{
            nullptr, vsg::ref_ptr(this)});
    }

    if (trajectory == nullptr)
    {
        return;
    }

    const double length = trajectory->getLength();

    begin_m = std::max(0.0, std::min(begin_m, length));
    end_m = std::max(0.0, std::min(end_m, length));

    if (end_m < begin_m)
    {
        std::swap(begin_m, end_m);
    }

    if (end_m - begin_m < 0.5)
    {
        return;
    }

    // Точки полосы каждые 5 м (минимум начало и конец)
    const std::size_t points_size = std::max<std::size_t>(
                2, static_cast<std::size_t>((end_m - begin_m) / 5.0) + 1);

    const auto vertices = vsg::vec3Array::create(points_size);
    const auto colors = vsg::vec3Array::create(points_size);
    const auto indices = vsg::ushortArray::create(points_size);

    for (std::size_t i = 0; i < points_size; ++i)
    {
        const double coord = begin_m +
                (end_m - begin_m) * static_cast<double>(i) /
                static_cast<double>(points_size - 1);

        const auto point = to_vsg_vec3(trajectory->getPosition(coord, 1).position);

        vertices->at(i) = vsg::vec3(point + vsg::dvec3{0.0, 0.0, 0.6});

        // Оранжевая полоса стройки
        colors->at(i) = {1.0f, 0.55f, 0.0f};
        indices->at(i) = static_cast<unsigned short>(i);
    }

    const auto geometry = vsg::Geometry::create();
    geometry->assignArrays(vsg::DataList{vertices, colors});
    geometry->assignIndices(indices);
    geometry->commands.push_back(vsg::DrawIndexed::create(
        points_size, 1, 0, 0, 0
    ));

    preview_switch->addChild(vsg::MASK_ALL, geometry);

    context_.compile_infos.emplace_back(CompileInfo{
        preview_switch, geometry, vsg::MASK_ALL});
}

void Route::hide_build_preview()
{
    const auto preview_switch = context_.build_preview_switch;

    if (!preview_switch || preview_switch->children.empty())
    {
        return;
    }

    preview_switch->children.clear();

    context_.compile_infos.emplace_back(CompileInfo{
        nullptr, vsg::ref_ptr(this)});
}

vsg::ref_ptr<vsg::StateGroup> create_trajectory_lines_state_group(
    vsg::ref_ptr<const vsg::Options> options)
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string shaders_dir_path =
        fs.getDataDir() + fs.separator() + "shaders";

    const auto input_assembly_state = vsg::InputAssemblyState::create();
    input_assembly_state->topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;

    return create_state_group_with_custom_pipeline(
        shaders_dir_path.c_str(),
        "traj_line.vert",
        "traj_line.frag",
        options,
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
}
