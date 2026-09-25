#include "editor/TrackFurniture.h"

#include "editor/EditorContext.h"
#include "editor/Mask.h"
#include "editor/Route.h"
#include "editor/SingleSwitch.h"
#include "editor/SplineTool.h"
#include "editor/TrackProfile.h"

#include <Journal.h>

#include <topology-defines.h>
#include <topology.h>
#include <trajectory.h>

#include <profile-point.h>
#include <vec3.h>

#include <vsg/core/Array.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ShaderSet.h>

#include <QString>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <random>
#include <utility>
#include <vector>

/// Смещение опоры КС от оси пути, м (промт п.9)
static constexpr double POLE_OFFSET = 3.5;

/// Высота опоры КС, м
static constexpr double POLE_HEIGHT = 7.0;

/// Радиус столба опоры КС, м
static constexpr double POLE_RADIUS = 0.12;

/// Длина консоли опоры (от столба к оси пути), м
static constexpr double CONSOLE_LENGTH = 3.0;

/// Смещение километрового столбика от оси пути, м
static constexpr double KM_POST_OFFSET = 2.5;

/// Шаг километровых столбиков, м
static constexpr double KM_POST_STEP = 1000.0;

/// Шаг сегментов платформы, м (промт п.10, упрощённо)
static constexpr double PLATFORM_SEGMENT_STEP = 10.0;

/// Отступ края платформы от оси пути, м
static constexpr double PLATFORM_MARGIN = 1.75;

/// Минимальный шаг опор КС, м
static constexpr double POLE_STEP_MIN = 5.0;

/// Шаг сэмплирования оси пути для рельсов и балласта, м
/// (кап производительности: рельс-сегменты не чаще 5 м)
static constexpr double TRACK_SEGMENT_STEP = 5.0;

/// Шаг сегментов терраформинга (насыпь/выемка/канава), м
static constexpr double TERRAIN_SEGMENT_STEP = 5.0;

/// Заложение откоса насыпи 1:1.5 (горизонтали на единицу высоты)
static constexpr double EMBANKMENT_SLOPE = 1.5;

/// Число боксов-ступеней откоса насыпи на каждую сторону
static constexpr std::size_t EMBANKMENT_STEPS = 3;

/// Шаг рельсовых стыков (жёлтые поперечины), м (промт п.12)
static constexpr double JOINT_STEP = 25.0;

/// Полуширина колеи: междурельсовое расстояние 1520 мм, м
static constexpr double RAIL_HALF_GAUGE = 0.76;

/// Габариты шпалы, м: длина (поперёк пути) x ширина (вдоль) x высота
static constexpr double SLEEPER_LENGTH = 2.5;
static constexpr double SLEEPER_WIDTH = 0.24;
static constexpr double SLEEPER_HEIGHT = 0.18;

/// Номинальный шаг шпал, м
static constexpr double SLEEPER_STEP = 0.54;

/// Максимум шпал на один вызов генерации (кап производительности:
/// сверх лимита шаг увеличивается)
static constexpr std::size_t SLEEPER_LIMIT = 4000;

/// Ширина балластной призмы, м
static constexpr double BALLAST_WIDTH = 4.0;

/// Высота балластной призмы, м
static constexpr double BALLAST_HEIGHT = 0.30;

/// Сечение рельса, м (по варианту из TrackProfile.rail_variant)
struct RailDimensions
{
    double width = 0.156;
    double height = 0.075;
};

/// Точка оси пути с базисом (результат getPosition)
struct PathSample
{
    vsg::dvec3 position = {0.0, 0.0, 0.0};
    vsg::dvec3 right = {0.0, 0.0, 0.0};
    vsg::dvec3 orth = {0.0, 1.0, 0.0};
};

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

/// Поворот геометрии по умолчанию (вдоль +Z) к заданному направлению
/// (по образцу Gizmo.cpp, направление нормализуется на входе)
static vsg::mat4 rotate_to_direction(vsg::vec3 direction)
{
    const float direction_length = vsg::length(direction);

    if (direction_length < 1.0e-6f)
    {
        return vsg::mat4();
    }

    direction /= direction_length;

    const vsg::vec3 axis = vsg::cross(vsg::vec3{0.0f, 0.0f, 1.0f},
        direction);

    if (vsg::length(axis) < 1.0e-6f)
    {
        // Направление совпадает с +Z или противоположно ему
        if (direction.z < 0.0f)
        {
            return vsg::rotate(static_cast<float>(vsg::PI),
                vsg::vec3{1.0f, 0.0f, 0.0f});
        }

        return vsg::mat4();
    }

    const float angle = std::acos(std::clamp(
        vsg::dot(vsg::vec3{0.0f, 0.0f, 1.0f}, direction), -1.0f, 1.0f));

    return vsg::rotate(angle, vsg::normalize(axis));
}

/// Настройка vsg::Builder-а на flat shaderSet сцены (как в Gizmo)
static void setup_builder(vsg::Builder& builder, const EditorContext& context)
{
    const auto shader_it = context.options->shaderSets.find("flat");

    if (shader_it != context.options->shaderSets.end())
    {
        builder.shaderSet = shader_it->second;
    }
    else
    {
        builder.shaderSet = vsg::createFlatShadedShaderSet();
    }
}

/// Точки оси пути с заданным шагом (сэмплы.getPosition под topology_mutex).
/// Конечная точка интервала включается всегда
static std::vector<PathSample> sample_trajectory(EditorContext& context,
    const std::string& trajectory_name, double begin, double end,
    double step)
{
    std::vector<PathSample> samples;

    if (trajectory_name.empty() || step < 0.1)
    {
        return samples;
    }

    std::lock_guard<std::mutex> lock_guard(context.topology_mutex);

    if (!context.topology)
    {
        return samples;
    }

    const traj_list_t* const traj_list =
        context.topology->getTrajectoriesList();

    const auto traj_it = traj_list->find(
        QString::fromStdString(trajectory_name));

    if (traj_it == traj_list->cend())
    {
        return samples;
    }

    const Trajectory* const trajectory = traj_it.value();

    end = std::min(end, trajectory->getLength());

    const auto append_sample = [&samples, trajectory](double coord) -> void
    {
        const profile_point_t point = trajectory->getPosition(coord, 1);
        samples.push_back(PathSample{to_vsg_vec3(point.position),
            to_vsg_vec3(point.right), to_vsg_vec3(point.orth)});
    };

    for (double coord = begin; coord < end; coord += step)
    {
        append_sample(coord);
    }

    append_sample(end);

    return samples;
}

/// Поиск метки модели в objects.ref по подстрокам: ключевые слова
/// перебираются от специфичного к общему, побеждает первый кандидат
static std::string find_model_label(const EditorContext& context,
    const std::vector<std::string>& keywords)
{
    for (const std::string& keyword : keywords)
    {
        if (keyword.empty())
        {
            continue;
        }

        for (const auto& [label, ref] : context.objects_ref)
        {
            std::string lowered = label;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                [](unsigned char c) -> char {
                    return static_cast<char>(std::tolower(c));
                });

            if (lowered.find(keyword) != std::string::npos)
            {
                return label;
            }
        }
    }

    return std::string();
}

/// Обёртка PagedLOD модели из objects.ref - как в RouteObject
/// (маски сцены и клика)
static vsg::ref_ptr<vsg::Node> create_model_node(EditorContext& context,
    const std::string& label)
{
    const auto ref_it = context.objects_ref.find(label);

    if (ref_it == context.objects_ref.end() || !ref_it->second.paged_lod)
    {
        return {};
    }

    return SingleSwitch::create(
        vsg::Mask{editor2::MASK_SCENE | editor2::MASK_CLICKABLE},
        ref_it->second.paged_lod);
}

/// Поворот вокруг вертикали: локальная ось +Y модели вдоль пути
static vsg::dmat4 yaw_to_track(const PathSample& sample)
{
    const double yaw = std::atan2(-sample.orth.x, sample.orth.y);

    return vsg::rotate(yaw, vsg::dvec3{0.0, 0.0, 1.0});
}

/// Столб опоры КС: вертикальный цилиндр h=7 м + консоль-бокс
/// от вершины столба к оси пути (локальные координаты узла)
static vsg::ref_ptr<vsg::Node> create_pole_geometry(
    vsg::Builder& builder, const PathSample& sample)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    // Столб: вертикальный цилиндр от земли
    const float radius = static_cast<float>(POLE_RADIUS);
    const float height = static_cast<float>(POLE_HEIGHT);

    vsg::box pole_box = {
        vsg::vec3(-radius, -radius, 0.0f),
        vsg::vec3( radius,  radius, height)
    };

    vsg::GeometryInfo pole_info(pole_box);
    pole_info.color = vsg::vec4{0.45f, 0.45f, 0.48f, 1.0f};

    const auto pole = builder.createCylinder(pole_info, state_info);

    // Консоль: горизонтальный бокс от вершины столба к оси пути
    // (опора стоит на стороне right, консоль смотрит на путь)
    const vsg::vec3 to_track = vsg::vec3{
        -static_cast<float>(sample.right.x),
        -static_cast<float>(sample.right.y),
        -static_cast<float>(sample.right.z)};

    const float console_len = static_cast<float>(CONSOLE_LENGTH);
    const float console_th = 0.08f;

    vsg::box console_box = {
        vsg::vec3(-console_th, -console_th, 0.0f),
        vsg::vec3( console_th,  console_th, console_len)
    };

    vsg::GeometryInfo console_info(console_box);
    console_info.color = vsg::vec4{0.30f, 0.30f, 0.33f, 1.0f};
    console_info.transform = vsg::translate(0.0f, 0.0f, height - 0.3f) *
        rotate_to_direction(to_track);

    const auto console = builder.createBox(console_info, state_info);

    const auto group = vsg::Group::create();
    group->addChild(pole);
    group->addChild(console);

    return group;
}

/// Километровый столбик: маленький цилиндр + табличка
static vsg::ref_ptr<vsg::Node> create_km_post_geometry(
    vsg::Builder& builder, const PathSample& sample)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    // Столбик: вертикальный цилиндр h=1.5 м
    const float radius = 0.06f;
    const float height = 1.5f;

    vsg::box post_box = {
        vsg::vec3(-radius, -radius, 0.0f),
        vsg::vec3( radius,  radius, height)
    };

    vsg::GeometryInfo post_info(post_box);
    post_info.color = vsg::vec4{0.85f, 0.85f, 0.85f, 1.0f};

    const auto post = builder.createCylinder(post_info, state_info);

    // Табличка: бокс вдоль пути (нормаль смотрит от пути)
    const vsg::vec3 along_track = vsg::vec3{
        static_cast<float>(sample.orth.x),
        static_cast<float>(sample.orth.y),
        static_cast<float>(sample.orth.z)};

    vsg::box plate_box = {
        vsg::vec3(-0.25f, -0.02f, -0.15f),
        vsg::vec3( 0.25f,  0.02f,  0.15f)
    };

    vsg::GeometryInfo plate_info(plate_box);
    plate_info.color = vsg::vec4{0.95f, 0.95f, 0.90f, 1.0f};
    plate_info.transform = vsg::translate(0.0f, 0.0f, height - 0.1f) *
        rotate_to_direction(along_track);

    const auto plate = builder.createBox(plate_info, state_info);

    const auto group = vsg::Group::create();
    group->addChild(post);
    group->addChild(plate);

    return group;
}

/// Платформа-лента: боксы между соседними точками оси пути
/// (геометрия строится в мировых координатах)
static vsg::ref_ptr<vsg::Node> create_platform_geometry(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double width, double height, bool right_side)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    const double side_sign = right_side ? 1.0 : -1.0;

    const float half_width = static_cast<float>(width * 0.5);
    const float half_height = static_cast<float>(height * 0.5);

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        const vsg::dvec3 direction = b.position - a.position;
        const double segment_length = vsg::length(direction);

        if (segment_length < 1.0e-6)
        {
            continue;
        }

        // Середина сегмента + боковое смещение от оси пути:
        // край платформы в PLATFORM_MARGIN от оси
        const vsg::dvec3 middle = (a.position + b.position) * 0.5;
        const vsg::dvec3 side_offset = (a.right + b.right) * 0.5 *
            side_sign * (PLATFORM_MARGIN + width * 0.5);

        vsg::box segment_box = {
            vsg::vec3(-half_width, -half_height, 0.0f),
            vsg::vec3( half_width,  half_height,
                static_cast<float>(segment_length))
        };

        vsg::GeometryInfo segment_info(segment_box);
        segment_info.color = vsg::vec4{0.55f, 0.53f, 0.50f, 1.0f};
        segment_info.transform =
            vsg::translate(vsg::vec3{
                static_cast<float>(middle.x + side_offset.x),
                static_cast<float>(middle.y + side_offset.y),
                static_cast<float>(middle.z + side_offset.z - height * 0.5)})
            * rotate_to_direction(vsg::vec3{
                static_cast<float>(direction.x),
                static_cast<float>(direction.y),
                static_cast<float>(direction.z)});

        group->addChild(builder.createBox(segment_info, state_info));
    }

    return group;
}

/// Сечение рельса по варианту из TrackProfile.rail_variant:
/// Р50 0.152x0.072 м, Р65 0.156x0.075, Р75 0.192x0.075
static RailDimensions rail_dimensions(const std::string& rail_variant)
{
    if (rail_variant == "Р50")
    {
        return RailDimensions{0.152, 0.072};
    }

    if (rail_variant == "Р75")
    {
        return RailDimensions{0.192, 0.075};
    }

    // Р65 и всё нераспознанное
    return RailDimensions{0.156, 0.075};
}

/// Рельсовая полоса: сегменты-боксы между соседними точками оси пути
/// со смещением offset от оси (головка рельса на уровне точки оси,
/// геометрия строится в мировых координатах - как у платформы)
static vsg::ref_ptr<vsg::Node> create_rail_strip(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double offset, const RailDimensions& rail)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    const float half_w = static_cast<float>(rail.width * 0.5);
    const float half_h = static_cast<float>(rail.height * 0.5);

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        // Точки рельса: точка оси + right * offset
        const vsg::dvec3 a_rail = a.position + a.right * offset;
        const vsg::dvec3 b_rail = b.position + b.right * offset;

        const vsg::dvec3 direction = b_rail - a_rail;
        const double segment_length = vsg::length(direction);

        if (segment_length < 1.0e-6)
        {
            continue;
        }

        // Середина сегмента, опущенная на половину высоты рельса
        const vsg::dvec3 middle = (a_rail + b_rail) * 0.5 -
            vsg::dvec3{0.0, 0.0, rail.height * 0.5};

        vsg::box segment_box = {
            vsg::vec3(-half_w, -half_h, 0.0f),
            vsg::vec3( half_w,  half_h,
                static_cast<float>(segment_length))
        };

        vsg::GeometryInfo segment_info(segment_box);
        segment_info.color = vsg::vec4{0.25f, 0.25f, 0.25f, 1.0f};
        segment_info.transform =
            vsg::translate(vsg::vec3{
                static_cast<float>(middle.x),
                static_cast<float>(middle.y),
                static_cast<float>(middle.z)}) *
            rotate_to_direction(vsg::vec3{
                static_cast<float>(direction.x),
                static_cast<float>(direction.y),
                static_cast<float>(direction.z)});

        group->addChild(builder.createBox(segment_info, state_info));
    }

    return group;
}

/// Шпалы: бокс 2.5x0.24x0.18 м в каждой точке сэмплирования,
/// поворот вокруг вертикали по направлению пути (yaw_to_track);
/// верх шпалы - под подошвой рельса
static vsg::ref_ptr<vsg::Node> create_sleepers_strip(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double rail_height, const std::string& sleeper_variant)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    // Ж/б шпала - серая, деревянная - коричневая
    const vsg::vec4 color = (sleeper_variant == "жб")
        ? vsg::vec4{0.5f, 0.5f, 0.5f, 1.0f}
        : vsg::vec4{0.4f, 0.28f, 0.15f, 1.0f};

    const float half_len = static_cast<float>(SLEEPER_LENGTH * 0.5);
    const float half_wid = static_cast<float>(SLEEPER_WIDTH * 0.5);
    const float height = static_cast<float>(SLEEPER_HEIGHT);

    for (const PathSample& sample : samples)
    {
        // Локальная ось +Y бокса разворачивается вдоль пути,
        // длинная сторона (2.5 м) ложится поперёк
        vsg::box sleeper_box = {
            vsg::vec3(-half_len, -half_wid, 0.0f),
            vsg::vec3( half_len,  half_wid, height)
        };

        vsg::GeometryInfo sleeper_info(sleeper_box);
        sleeper_info.color = color;
        sleeper_info.transform =
            vsg::translate(vsg::vec3{
                static_cast<float>(sample.position.x),
                static_cast<float>(sample.position.y),
                static_cast<float>(sample.position.z - rail_height -
                    SLEEPER_HEIGHT)}) *
            vsg::mat4(yaw_to_track(sample));

        group->addChild(builder.createBox(sleeper_info, state_info));
    }

    return group;
}

/// Балластная призма: лента шириной 4.0 м высотой 0.30 м между
/// точками оси (как платформа, но без бокового смещения);
/// верх призмы - под подошвой шпал
static vsg::ref_ptr<vsg::Node> create_ballast_strip(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double rail_height)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    const float half_width = static_cast<float>(BALLAST_WIDTH * 0.5);
    const float half_height = static_cast<float>(BALLAST_HEIGHT * 0.5);

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        const vsg::dvec3 direction = b.position - a.position;
        const double segment_length = vsg::length(direction);

        if (segment_length < 1.0e-6)
        {
            continue;
        }

        const vsg::dvec3 middle = (a.position + b.position) * 0.5;

        vsg::box segment_box = {
            vsg::vec3(-half_width, -half_height, 0.0f),
            vsg::vec3( half_width,  half_height,
                static_cast<float>(segment_length))
        };

        vsg::GeometryInfo segment_info(segment_box);
        segment_info.color = vsg::vec4{0.35f, 0.32f, 0.28f, 1.0f};
        segment_info.transform =
            vsg::translate(vsg::vec3{
                static_cast<float>(middle.x),
                static_cast<float>(middle.y),
                static_cast<float>(middle.z - rail_height -
                    SLEEPER_HEIGHT - BALLAST_HEIGHT * 0.5)}) *
            rotate_to_direction(vsg::vec3{
                static_cast<float>(direction.x),
                static_cast<float>(direction.y),
                static_cast<float>(direction.z)});

        group->addChild(builder.createBox(segment_info, state_info));
    }

    return group;
}

/// Стыки рельсов: тонкая жёлтая поперечина 0.02 м между рельсами
/// (верх - на уровне головки рельса), поворот вокруг вертикали
/// по направлению пути (yaw_to_track)
static vsg::ref_ptr<vsg::Node> create_joints_strip(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    const RailDimensions& rail)
{
    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    // Поперечина накрывает головки обоих рельсов
    const float half_len = static_cast<float>(RAIL_HALF_GAUGE +
        rail.width * 0.5);

    for (const PathSample& sample : samples)
    {
        vsg::box joint_box = {
            vsg::vec3(-half_len, -0.01f, 0.0f),
            vsg::vec3( half_len,  0.01f, 0.02f)
        };

        vsg::GeometryInfo joint_info(joint_box);
        joint_info.color = vsg::vec4{1.0f, 0.9f, 0.1f, 1.0f};
        joint_info.transform =
            vsg::translate(vsg::vec3{
                static_cast<float>(sample.position.x),
                static_cast<float>(sample.position.y),
                static_cast<float>(sample.position.z - 0.02f)}) *
            vsg::mat4(yaw_to_track(sample));

        group->addChild(builder.createBox(joint_info, state_info));
    }

    return group;
}

/// Бокс-лента вдоль сегмента пути: ориентация по направлению
/// сегмента (rotate_to_direction), центр смещён от оси пути
/// на lateral_center м, вертикальный диапазон [bottom_z, top_z]
/// (геометрия в мировых координатах - как у платформы)
static void add_terrain_segment_box(vsg::Builder& builder,
    vsg::Group& group, const PathSample& a, const PathSample& b,
    double lateral_center, double half_width, double top_z,
    double bottom_z, const vsg::vec4& color)
{
    const double height = top_z - bottom_z;

    if (height < 1.0e-6)
    {
        return;
    }

    const vsg::dvec3 direction = b.position - a.position;
    const double segment_length = vsg::length(direction);

    if (segment_length < 1.0e-6)
    {
        return;
    }

    // Середина сегмента + боковое смещение от оси пути
    const vsg::dvec3 middle = (a.position + b.position) * 0.5 +
        (a.right + b.right) * 0.5 * lateral_center;

    const float half_w = static_cast<float>(half_width);
    const float half_h = static_cast<float>(height * 0.5);

    vsg::box segment_box = {
        vsg::vec3(-half_w, -half_h, 0.0f),
        vsg::vec3( half_w,  half_h,
            static_cast<float>(segment_length))
    };

    vsg::StateInfo state_info;
    state_info.two_sided = true;

    vsg::GeometryInfo segment_info(segment_box);
    segment_info.color = color;
    segment_info.transform =
        vsg::translate(vsg::vec3{
            static_cast<float>(middle.x),
            static_cast<float>(middle.y),
            static_cast<float>((top_z + bottom_z) * 0.5)}) *
        rotate_to_direction(vsg::vec3{
            static_cast<float>(direction.x),
            static_cast<float>(direction.y),
            static_cast<float>(direction.z)});

    group.addChild(builder.createBox(segment_info, state_info));
}

/// Насыпь: трапецеидальная земляная лента вдоль пути - верхнее
/// полотно шириной shoulder м и ступенчатые откосы 1:1.5 по обе
/// стороны, спускающиеся на height м от уровня пути
static vsg::ref_ptr<vsg::Node> create_embankment_geometry(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double height, double shoulder)
{
    const auto group = vsg::Group::create();

    // Цвет земли (промт: терраформинг вдоль пути)
    const vsg::vec4 earth_color{0.40f, 0.33f, 0.25f, 1.0f};

    // Ширина одной ступени откоса: заложение 1:1.5 на высоту height
    const double step_width = height * EMBANKMENT_SLOPE /
        static_cast<double>(EMBANKMENT_STEPS);

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        const double track_z = (a.position.z + b.position.z) * 0.5;

        // Верхнее полотно насыпи под подошвой балласта
        add_terrain_segment_box(builder, *group, a, b, 0.0,
            shoulder * 0.5, track_z, track_z - height, earth_color);

        // Ступени откосов: каждая следующая ступень ниже
        // и дальше от оси (аппроксимация откоса 1:1.5)
        for (const double side_sign : {-1.0, 1.0})
        {
            for (std::size_t step = 0; step < EMBANKMENT_STEPS; ++step)
            {
                const double inner = shoulder * 0.5 +
                    static_cast<double>(step) * step_width;

                const double top = track_z - height *
                    static_cast<double>(step) /
                    static_cast<double>(EMBANKMENT_STEPS);

                add_terrain_segment_box(builder, *group, a, b,
                    side_sign * (inner + step_width * 0.5),
                    step_width * 0.5, top, track_z - height,
                    earth_color);
            }
        }
    }

    return group;
}

/// Выемка: приподнятые земляные стенки по краям пути (width/2 от
/// оси, толщина по заложению 1:1) на глубину depth м
static vsg::ref_ptr<vsg::Node> create_cutting_geometry(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    double depth, double width)
{
    const auto group = vsg::Group::create();

    const vsg::vec4 earth_color{0.40f, 0.33f, 0.25f, 1.0f};

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        const double track_z = (a.position.z + b.position.z) * 0.5;

        // Стенки поднимаются на depth над уровнем пути, снизу
        // опущены на 0.5 м ниже пути (визуальная толщина)
        for (const double side_sign : {-1.0, 1.0})
        {
            const double inner = width * 0.5;

            add_terrain_segment_box(builder, *group, a, b,
                side_sign * (inner + depth * 0.5), depth * 0.5,
                track_z + depth, track_z - 0.5, earth_color);
        }
    }

    return group;
}

/// Канава-кювет: узкий заглублённый бокс на offset width/2+2.0 м
/// от оси выбранной стороны (тёмный цвет)
static vsg::ref_ptr<vsg::Node> create_ditch_geometry(
    vsg::Builder& builder, const std::vector<PathSample>& samples,
    bool right_side, double width, double depth)
{
    const auto group = vsg::Group::create();

    const double side_sign = right_side ? 1.0 : -1.0;

    const double offset = width * 0.5 + 2.0;

    const vsg::vec4 dark_color{0.25f, 0.19f, 0.13f, 1.0f};

    for (std::size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const PathSample& a = samples[i];
        const PathSample& b = samples[i + 1];

        const double track_z = (a.position.z + b.position.z) * 0.5;

        // Верх канавы чуть ниже уровня головки рельса
        add_terrain_segment_box(builder, *group, a, b,
            side_sign * offset, width * 0.5, track_z - 0.2,
            track_z - 0.2 - depth, dark_color);
    }

    return group;
}

/// Сэмплы пути из вектора позиций: направление (orth) и правая
/// сторона (right = cross(orth, вертикаль), как в track_t::
/// calc_parameters) выводятся из соседних точек
static std::vector<PathSample> samples_from_positions(
    const std::vector<vsg::dvec3>& positions)
{
    std::vector<PathSample> samples;
    samples.reserve(positions.size());

    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        const vsg::dvec3& prev =
            positions[i > 0 ? i - 1 : i];
        const vsg::dvec3& next =
            positions[i + 1 < positions.size() ? i + 1 : i];

        PathSample sample;
        sample.position = positions[i];

        const vsg::dvec3 direction = next - prev;

        if (vsg::length(direction) > 1.0e-6)
        {
            sample.orth = vsg::normalize(direction);

            const vsg::dvec3 right = vsg::cross(sample.orth,
                vsg::dvec3{0.0, 0.0, 1.0});

            if (vsg::length(right) > 1.0e-6)
            {
                sample.right = vsg::normalize(right);
            }
            else if (!samples.empty())
            {
                // Вертикальный участок: сторона от предыдущей точки
                sample.right = samples.back().right;
            }
            else
            {
                sample.right = vsg::dvec3{1.0, 0.0, 0.0};
            }
        }
        else if (!samples.empty())
        {
            sample.orth = samples.back().orth;
            sample.right = samples.back().right;
        }
        else
        {
            sample.orth = vsg::dvec3{0.0, 1.0, 0.0};
            sample.right = vsg::dvec3{1.0, 0.0, 0.0};
        }

        samples.push_back(sample);
    }

    return samples;
}

/// Длина полилинии по точкам, м
static double polyline_length(const std::vector<vsg::dvec3>& points)
{
    double total_length = 0.0;

    for (std::size_t i = 1; i < points.size(); ++i)
    {
        total_length += vsg::length(points[i] - points[i - 1]);
    }

    return total_length;
}

/// Точки вдоль полилинии с равномерным шагом по её длине
/// (конечная точка включается всегда)
static std::vector<vsg::dvec3> resample_by_step(
    const std::vector<vsg::dvec3>& points, double step)
{
    if (points.size() < 2 || step < 1.0e-6)
    {
        return points;
    }

    std::vector<vsg::dvec3> result;
    result.push_back(points.front());

    double rest = step;

    for (std::size_t i = 1; i < points.size(); ++i)
    {
        const vsg::dvec3 segment = points[i] - points[i - 1];
        const double segment_length = vsg::length(segment);

        if (segment_length < 1.0e-9)
        {
            continue;
        }

        const vsg::dvec3 direction = segment / segment_length;

        double passed = 0.0;

        while (segment_length - passed >= rest)
        {
            passed += rest;
            result.push_back(points[i - 1] + direction * passed);
            rest = step;
        }

        rest -= segment_length - passed;
    }

    // Конечная точка включается всегда
    if (vsg::length(points.back() - result.back()) > 1.0e-6)
    {
        result.push_back(points.back());
    }

    return result;
}

/// Общая сборка геометрии верхнего строения пути по вектору позиций
/// оси (мировые координаты): рельсы по варианту профиля, шпалы
/// и балласт с учётом скрытия мешей, стыки по флагу mark_joints.
/// sleeper_count - число уложенных шпал, sleeper_step_increased -
/// превышен кап 4000 шт. Используется и для траекторий, и для
/// сплайнов новых путей (инструмент «Новый путь»)
vsg::ref_ptr<vsg::Node> TrackFurniture::build_track_geometry(
    EditorContext& context, const std::vector<vsg::dvec3>& points,
    const TrackProfile& profile, std::size_t* sleeper_count,
    bool* sleeper_step_increased)
{
    if (sleeper_count != nullptr)
    {
        *sleeper_count = 0;
    }

    if (sleeper_step_increased != nullptr)
    {
        *sleeper_step_increased = false;
    }

    if (points.size() < 2)
    {
        return {};
    }

    const double total_length = polyline_length(points);

    if (total_length < 1.0e-6)
    {
        return {};
    }

    const auto mesh = vsg::Group::create();

    // Рельсы и балласт: точки оси каждые 5 м
    const std::vector<PathSample> rail_samples = samples_from_positions(
        resample_by_step(points, TRACK_SEGMENT_STEP));

    const RailDimensions rail = rail_dimensions(profile.rail_variant);

    vsg::Builder builder;
    setup_builder(builder, context);

    if (!profile.rail_hidden)
    {
        // Две рельсовые полосы на междурельсовом 1520 мм
        mesh->addChild(create_rail_strip(builder, rail_samples,
            -RAIL_HALF_GAUGE, rail));
        mesh->addChild(create_rail_strip(builder, rail_samples,
            RAIL_HALF_GAUGE, rail));
    }

    if (!profile.ballast_hidden)
    {
        mesh->addChild(create_ballast_strip(builder, rail_samples,
            rail.height));
    }

    if (!profile.sleeper_hidden)
    {
        // Кап производительности: не более 4000 шпал на вызов,
        // сверх лимита шаг шпал увеличивается
        double sleeper_step = SLEEPER_STEP;

        if (total_length / SLEEPER_STEP >
            static_cast<double>(SLEEPER_LIMIT))
        {
            sleeper_step = total_length /
                static_cast<double>(SLEEPER_LIMIT);

            if (sleeper_step_increased != nullptr)
            {
                *sleeper_step_increased = true;
            }
        }

        const std::vector<PathSample> sleeper_samples =
            samples_from_positions(resample_by_step(points,
                sleeper_step));

        if (sleeper_count != nullptr)
        {
            *sleeper_count = sleeper_samples.size();
        }

        mesh->addChild(create_sleepers_strip(builder, sleeper_samples,
            rail.height, profile.sleeper_variant));
    }

    // Стыки рельсов: жёлтая поперечина каждые 25 м (промт п.12)
    if (profile.mark_joints)
    {
        const std::vector<PathSample> joint_samples =
            samples_from_positions(resample_by_step(points, JOINT_STEP));

        mesh->addChild(create_joints_strip(builder, joint_samples, rail));
    }

    return mesh;
}

/// Собрать геометрию верхнего строения пути на интервале
/// [from, to] м траектории: сэмплы оси через getPosition
/// (мировые координаты). sleeper_count - число уложенных шпал,
/// sleeper_step_increased - превышен кап 4000 шт.
static vsg::ref_ptr<vsg::Node> create_track_mesh_geometry(
    EditorContext& context, const std::string& trajectory_name,
    double from, double to, const TrackProfile& profile,
    std::size_t& sleeper_count, bool& sleeper_step_increased)
{
    sleeper_count = 0;
    sleeper_step_increased = false;

    // Точки оси пути каждые 5 м
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, from, to, TRACK_SEGMENT_STEP);

    if (samples.size() < 2)
    {
        return {};
    }

    std::vector<vsg::dvec3> positions;
    positions.reserve(samples.size());

    for (const PathSample& sample : samples)
    {
        positions.push_back(sample.position);
    }

    // Общая сборка геометрии (общая и для сплайнов новых путей)
    return TrackFurniture::build_track_geometry(context, positions,
        profile, &sleeper_count, &sleeper_step_increased);
}

/// Собрать геометрию деревьев вдоль всей траектории (мировые
/// координаты): точки с шагом 1000/per_km м, отступ и габариты
/// каждого дерева - детерминированный ПСЧ с seed из имени
/// траектории (при восстановлении из конфига лес совпадает).
///
/// Инстансинг растительности (vsg::Builder, сверен с vsg/utils/
/// Builder.h): StateInfo::instance_positions_vec3 + GeometryInfo::
/// positions (vec3Array) дают один instanced меш вместо отдельного
/// на каждое дерево. Габариты деревьев квантуются в прототипы -
/// 6 уровней высоты ствола x 3 радиуса кроны: весь лес собирается
/// максимум 18 парами вызовов Builder (ствол+крона) вместо двух
/// мешей на каждое дерево.
/// TODO: поэкземплярные цвета (GeometryInfo::colors как vec4Array)
/// не используются - поддержка per-instance цветов Builder'ом
/// не подтверждена заголовком, оттенок кроны варьируется по группе
static vsg::ref_ptr<vsg::Node> create_trees_geometry(
    EditorContext& context, const std::string& trajectory_name,
    bool right_side, double per_km, double offset_min,
    double offset_max, std::size_t& tree_count)
{
    tree_count = 0;

    per_km = std::clamp(per_km, 1.0, 1000.0);
    offset_min = std::clamp(offset_min, 0.0, 100.0);
    offset_max = std::clamp(offset_max, offset_min, 200.0);

    // Точки оси пути с шагом 1000/per_km метров
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, 0.0, std::numeric_limits<double>::max(),
        1000.0 / per_km);

    if (samples.empty())
    {
        return {};
    }

    std::mt19937_64 rng(static_cast<std::uint64_t>(
        std::hash<std::string>{}(trajectory_name)));

    std::uniform_real_distribution<double> offset_dist(offset_min,
        offset_max);
    std::uniform_real_distribution<double> height_dist(3.0, 6.0);
    std::uniform_real_distribution<double> crown_dist(0.8, 1.4);

    const double side_sign = right_side ? 1.0 : -1.0;

    // Прототипы деревьев: квантованные высота ствола и радиус кроны.
    // Шаги квантования покрывают исходные диапазоны 3..6 м и 0.8..1.4 м
    constexpr std::size_t TRUNK_LEVELS = 6;
    constexpr std::size_t CROWN_LEVELS = 3;
    constexpr std::size_t TREE_GROUPS = TRUNK_LEVELS * CROWN_LEVELS;

    // Накопители позиций инстансов каждой группы
    std::array<std::vector<vsg::vec3>, TREE_GROUPS> trunk_positions;
    std::array<std::vector<vsg::vec3>, TREE_GROUPS> crown_positions;

    for (const PathSample& sample : samples)
    {
        const double offset = offset_dist(rng);
        const double trunk_height = height_dist(rng);
        const double crown_radius = crown_dist(rng);

        const vsg::dvec3 tree_pos = sample.position +
            sample.right * (side_sign * offset);

        // Группа прототипа: индекс уровня высоты ствола и радиуса кроны
        const std::size_t height_level = std::min(
            static_cast<std::size_t>((trunk_height - 3.0) / 0.5),
            TRUNK_LEVELS - 1);
        const std::size_t crown_level = std::min(
            static_cast<std::size_t>((crown_radius - 0.8) / 0.2),
            CROWN_LEVELS - 1);
        const std::size_t group = height_level * CROWN_LEVELS +
            crown_level;

        const vsg::vec3 translation{
            static_cast<float>(tree_pos.x),
            static_cast<float>(tree_pos.y),
            static_cast<float>(tree_pos.z)};

        trunk_positions[group].push_back(translation);
        crown_positions[group].push_back(translation);

        ++tree_count;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    vsg::StateInfo state_info;
    state_info.two_sided = true;

    // Инстансинг: массив позиций в GeometryInfo (vsg/utils/Builder.h:
    // instance_positions_vec3 требует vec3Array с позициями инстансов)
    state_info.instance_positions_vec3 = true;

    const auto group = vsg::Group::create();

    for (std::size_t g = 0; g < TREE_GROUPS; ++g)
    {
        if (trunk_positions[g].empty())
        {
            continue;
        }

        // Параметры прототипа: середина уровня квантования
        const std::size_t height_level = g / CROWN_LEVELS;
        const std::size_t crown_level = g % CROWN_LEVELS;

        const double trunk_height =
            3.0 + (static_cast<double>(height_level) + 0.5) * 0.5;
        const double crown_radius =
            0.8 + (static_cast<double>(crown_level) + 0.5) * 0.2;

        // Оттенок кроны - детерминированный по группе: зелёный
        // вариативный между группами прототипов
        const float shade =
            static_cast<float>(g % 5) / 4.0f;

        const vsg::vec4 crown_color{
            0.10f + 0.15f * shade,
            0.35f + 0.30f * shade,
            0.10f + 0.10f * shade,
            1.0f};

        // Ствол: цилиндр 0.25 м в диаметре, h 3-6 м. Геометрия
        // прототипа канонически центрирована вокруг точки постановки
        // инстанса (как у Builder: единичная форма симметрична
        // относительно GeometryInfo::position, а позиции инстансов
        // замещают её). Вертикальная центровка (середина ствола)
        // зашита в позиции инстансов, поэтому info.position = (0,0,0)
        const float trunk_r = 0.125f;

        vsg::GeometryInfo trunk_info;
        trunk_info.dx = vsg::vec3(2.0f * trunk_r, 0.0f, 0.0f);
        trunk_info.dy = vsg::vec3(0.0f, 2.0f * trunk_r, 0.0f);
        trunk_info.dz = vsg::vec3(0.0f, 0.0f,
            static_cast<float>(trunk_height));
        trunk_info.color = vsg::vec4{0.35f, 0.22f, 0.12f, 1.0f};

        // Инстанс ставится в середину ствола (низ — на земле)
        const vsg::vec3 trunk_center_offset(0.0f, 0.0f,
            static_cast<float>(trunk_height * 0.5));

        const auto trunk_instances = vsg::vec3Array::create(
            static_cast<uint32_t>(trunk_positions[g].size()));

        for (std::size_t s = 0; s < trunk_positions[g].size(); ++s)
        {
            trunk_instances->at(s) = trunk_positions[g][s] +
                trunk_center_offset;
        }

        trunk_info.positions = trunk_instances;

        group->addChild(builder.createCylinder(trunk_info, state_info));

        // Крона: конус над стволом (хвойное дерево), центрируется
        // в середине своего вертикального диапазона
        const float crown_r = static_cast<float>(crown_radius);
        const float crown_begin =
            static_cast<float>(trunk_height * 0.45);
        const float crown_h =
            static_cast<float>(trunk_height * 0.55 + 0.5);

        vsg::GeometryInfo crown_info;
        crown_info.dx = vsg::vec3(2.0f * crown_r, 0.0f, 0.0f);
        crown_info.dy = vsg::vec3(0.0f, 2.0f * crown_r, 0.0f);
        crown_info.dz = vsg::vec3(0.0f, 0.0f, crown_h);
        crown_info.color = crown_color;

        const vsg::vec3 crown_center_offset(0.0f, 0.0f,
            crown_begin + crown_h * 0.5f);

        const auto crown_instances = vsg::vec3Array::create(
            static_cast<uint32_t>(crown_positions[g].size()));

        for (std::size_t s = 0; s < crown_positions[g].size(); ++s)
        {
            crown_instances->at(s) = crown_positions[g][s] +
                crown_center_offset;
        }

        crown_info.positions = crown_instances;

        group->addChild(builder.createCone(crown_info, state_info));
    }

    return group;
}

/// Плоскость воды: полупрозрачный синий бокс (blending как
/// в Gizmo), зеркало на отметке level, геометрия в мировых
/// координатах
static vsg::ref_ptr<vsg::Node> create_water_geometry(
    EditorContext& context, const vsg::dvec3& center, double size_x,
    double size_y, double level)
{
    vsg::Builder builder;
    setup_builder(builder, context);

    vsg::StateInfo state_info;
    state_info.two_sided = true;
    state_info.blending = true;

    // Толщина плиты воды под зеркалом, м
    const float thickness = 0.5f;

    vsg::box water_box = {
        vsg::vec3(-static_cast<float>(size_x * 0.5),
            -static_cast<float>(size_y * 0.5),
            -thickness),
        vsg::vec3( static_cast<float>(size_x * 0.5),
             static_cast<float>(size_y * 0.5),
             0.0f)
    };

    vsg::GeometryInfo water_info(water_box);
    water_info.color = vsg::vec4{0.2f, 0.35f, 0.6f, 0.55f};
    water_info.transform = vsg::translate(vsg::vec3{
        static_cast<float>(center.x),
        static_cast<float>(center.y),
        static_cast<float>(level)});

    return builder.createBox(water_info, state_info);
}

/// Переезд на координате coord: настил 4 м вдоль пути x 8 м поперёк
/// + два столбика шлагбаума 0.15x0.15x3 м на ±4 м от оси
static vsg::ref_ptr<vsg::Node> create_crossing_geometry(
    EditorContext& context, const std::string& trajectory_name,
    double coord)
{
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, coord, coord, 1.0);

    if (samples.empty())
    {
        return {};
    }

    const PathSample& sample = samples.front();

    vsg::Builder builder;
    setup_builder(builder, context);

    vsg::StateInfo state_info;
    state_info.two_sided = true;

    const auto group = vsg::Group::create();

    // Настил: локальная ось +Y разворачивается вдоль пути
    // (yaw_to_track), верх на уровне оси пути
    vsg::box deck_box = {
        vsg::vec3(-4.0f, -2.0f, -0.15f),
        vsg::vec3( 4.0f,  2.0f,  0.0f)
    };

    vsg::GeometryInfo deck_info(deck_box);
    deck_info.color = vsg::vec4{0.30f, 0.27f, 0.24f, 1.0f};
    deck_info.transform = vsg::translate(vsg::vec3{
        static_cast<float>(sample.position.x),
        static_cast<float>(sample.position.y),
        static_cast<float>(sample.position.z)}) * vsg::mat4(yaw_to_track(sample));

    group->addChild(builder.createBox(deck_info, state_info));

    // Столбики шлагбаума на краях настила (±4 м от оси)
    for (const double side_sign : {-1.0, 1.0})
    {
        const vsg::dvec3 post_pos = sample.position +
            sample.right * (4.0 * side_sign);

        vsg::box post_box = {
            vsg::vec3(-0.075f, -0.075f, 0.0f),
            vsg::vec3( 0.075f,  0.075f, 3.0f)
        };

        vsg::GeometryInfo post_info(post_box);
        post_info.color = vsg::vec4{0.85f, 0.85f, 0.85f, 1.0f};
        post_info.transform = vsg::translate(vsg::vec3{
            static_cast<float>(post_pos.x),
            static_cast<float>(post_pos.y),
            static_cast<float>(post_pos.z)});

        group->addChild(builder.createBox(post_info, state_info));
    }

    return group;
}

/// Дорога у переезда: серая асфальтовая лента шириной 7 м
/// перпендикулярно пути (вдоль right) в обе стороны от переезда
/// на length/2 м (промт п.11, упрощённо)
static vsg::ref_ptr<vsg::Node> create_road_geometry(
    EditorContext& context, const std::string& trajectory_name,
    double coord, double length)
{
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, coord, coord, 1.0);

    if (samples.empty())
    {
        return {};
    }

    const PathSample& sample = samples.front();

    vsg::Builder builder;
    setup_builder(builder, context);

    vsg::StateInfo state_info;
    state_info.two_sided = true;

    // Локальная ось +Z разворачивается вдоль right (поперёк пути):
    // одна плита перекрывает обе стороны от переезда
    const float half_road_width = 3.5f;
    const float half_thickness = 0.1f;

    vsg::box road_box = {
        vsg::vec3(-half_road_width, -half_thickness,
            -static_cast<float>(length * 0.5)),
        vsg::vec3( half_road_width,  half_thickness,
             static_cast<float>(length * 0.5))
    };

    vsg::GeometryInfo road_info(road_box);
    road_info.color = vsg::vec4{0.2f, 0.2f, 0.2f, 1.0f};
    road_info.transform =
        vsg::translate(vsg::vec3{
            static_cast<float>(sample.position.x),
            static_cast<float>(sample.position.y),
            static_cast<float>(sample.position.z - half_thickness)}) *
        rotate_to_direction(vsg::vec3{
            static_cast<float>(sample.right.x),
            static_cast<float>(sample.right.y),
            static_cast<float>(sample.right.z)});

    return builder.createBox(road_info, state_info);
}

/// Добавить элемент обвеса: узел в MatrixTransform -> generated_group,
/// метаданные -> generated_items, узел -> очередь компиляции
static void add_generated_item(EditorContext& context,
    const GeneratedConfig& meta, vsg::ref_ptr<vsg::Node> node,
    const vsg::dmat4& matrix)
{
    if (!node || !context.generated_group)
    {
        return;
    }

    const auto transform = vsg::MatrixTransform::create();
    transform->matrix = matrix;
    transform->addChild(node);

    GeneratedItem item;
    item.node = transform;
    item.meta = meta;

    context.generated_items.push_back(std::move(item));

    context.compile_infos.emplace_back(CompileInfo{
        context.generated_group, transform, vsg::MASK_ALL});
}

/// Доступен ли обвес: группа сцены создана и топология загружена
static bool furniture_ready(const EditorContext& context)
{
    return context.generated_group != nullptr &&
        context.topology_loaded.load();
}

bool TrackFurniture::generate_catenary_poles(EditorContext& context,
    const std::string& trajectory_name, double step_m,
    double begin_m, double end_m)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    step_m = std::max(step_m, POLE_STEP_MIN);

    // Точки оси пути каждые step_m метров в заданном диапазоне
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, begin_m, end_m, step_m);

    if (samples.empty())
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    // Готовая модель опоры из objects.ref, иначе - столб vsg::Builder
    const std::string model_label = find_model_label(context,
        {"catenary", "ks_pole", "опора", "pole"});

    vsg::Builder builder;
    setup_builder(builder, context);

    std::size_t created = 0;

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        const PathSample& sample = samples[i];

        // Позиция: точка пути + смещение вправо на 3.5 м
        const vsg::dvec3 pole_pos = sample.position +
            sample.right * POLE_OFFSET;

        GeneratedConfig meta;
        meta.kind = "CatenaryPole";
        meta.trajectory = trajectory_name;
        meta.coord = static_cast<double>(i) * step_m;
        meta.side = "right";
        meta.label = model_label;

        if (!model_label.empty())
        {
            const auto model = create_model_node(context, model_label);

            if (model)
            {
                add_generated_item(context, meta, model,
                    vsg::translate(pole_pos) * yaw_to_track(sample));
                ++created;
                continue;
            }
        }

        add_generated_item(context, meta,
            create_pole_geometry(builder, sample),
            vsg::translate(pole_pos));
        ++created;
    }

    context.status = "Опоры КС: " + std::to_string(created) + " шт.";

    return created > 0;
}

bool TrackFurniture::generate_platform(EditorContext& context,
    const std::string& trajectory_name, double length_m, double width_m,
    double height_m, bool right_side, double begin_m)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    // Ограничения габаритов платформы (защита от опечаток в полях)
    length_m = std::clamp(length_m, 10.0, 10000.0);
    width_m = std::clamp(width_m, 1.0, 20.0);
    height_m = std::clamp(height_m, 0.1, 5.0);

    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, begin_m, begin_m + length_m, PLATFORM_SEGMENT_STEP);

    if (samples.size() < 2)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    const auto node = create_platform_geometry(builder, samples,
        width_m, height_m, right_side);

    GeneratedConfig meta;
    meta.kind = "Platform";
    meta.trajectory = trajectory_name;
    meta.coord = 0.0;
    meta.side = right_side ? "right" : "left";
    meta.length = length_m;
    meta.width = width_m;
    meta.height = height_m;

    // Геометрия платформы уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Платформа: " + std::to_string(samples.size() - 1) +
        " сегментов";

    return true;
}

bool TrackFurniture::generate_km_posts(EditorContext& context,
    const std::string& trajectory_name)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, 0.0, std::numeric_limits<double>::max(),
        KM_POST_STEP);

    if (samples.empty())
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    // Готовая модель столбика из objects.ref, иначе - vsg::Builder
    const std::string model_label = find_model_label(context,
        {"km_post", "picket", "километр", "столб"});

    vsg::Builder builder;
    setup_builder(builder, context);

    std::size_t created = 0;

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        const PathSample& sample = samples[i];

        const vsg::dvec3 post_pos = sample.position +
            sample.right * KM_POST_OFFSET;

        GeneratedConfig meta;
        meta.kind = "KmPost";
        meta.trajectory = trajectory_name;
        meta.coord = static_cast<double>(i) * KM_POST_STEP;
        meta.side = "right";
        meta.label = model_label;

        if (!model_label.empty())
        {
            const auto model = create_model_node(context, model_label);

            if (model)
            {
                add_generated_item(context, meta, model,
                    vsg::translate(post_pos) * yaw_to_track(sample));
                ++created;
                continue;
            }
        }

        add_generated_item(context, meta,
            create_km_post_geometry(builder, sample),
            vsg::translate(post_pos));
        ++created;
    }

    context.status = "Километровые столбики: " +
        std::to_string(created) + " шт.";

    return created > 0;
}

bool TrackFurniture::generate_track_mesh(EditorContext& context,
    const std::string& trajectory_name, double from, double to,
    const TrackProfile& profile)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    from = std::max(from, 0.0);

    if (to <= from)
    {
        context.status = "Путь: пустой интервал генерации";
        return false;
    }

    std::size_t sleeper_count = 0;
    bool sleeper_step_increased = false;

    const auto node = create_track_mesh_geometry(context, trajectory_name,
        from, to, profile, sleeper_count, sleeper_step_increased);

    if (!node)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    GeneratedConfig meta;
    meta.kind = "TrackMesh";
    meta.trajectory = trajectory_name;
    meta.from = from;
    meta.to = to;

    // Геометрия пути уже в мировых координатах (как у платформы)
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Путь: " + std::to_string(sleeper_count) + " шпал";

    if (sleeper_step_increased)
    {
        // Кап производительности: шаг шпал увеличен сверх 4000 шт.
        context.status += " (шаг увеличен: предел 4000 шпал)";
    }

    return true;
}

bool TrackFurniture::generate_trees_along(EditorContext& context,
    const std::string& trajectory_name, bool right_side, double per_km,
    double offset_min, double offset_max)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    std::size_t tree_count = 0;

    const auto node = create_trees_geometry(context, trajectory_name,
        right_side, per_km, offset_min, offset_max, tree_count);

    if (!node)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    GeneratedConfig meta;
    meta.kind = "Trees";
    meta.trajectory = trajectory_name;
    meta.side = right_side ? "right" : "left";
    meta.per_km = std::clamp(per_km, 1.0, 1000.0);
    meta.offset_min = std::clamp(offset_min, 0.0, 100.0);
    meta.offset_max = std::clamp(offset_max, meta.offset_min, 200.0);

    // Весь лес - одна запись (геометрия в мировых координатах)
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Деревья: " + std::to_string(tree_count) + " шт.";

    return true;
}

bool TrackFurniture::generate_water(EditorContext& context,
    const vsg::dvec3& center, double size_x, double size_y, double level)
{
    // Вода не привязана к топологии: достаточно группы сцены
    if (!context.generated_group)
    {
        context.status = "Обвес: маршрут не загружен";
        return false;
    }

    size_x = std::clamp(size_x, 1.0, 10000.0);
    size_y = std::clamp(size_y, 1.0, 10000.0);

    const auto node = create_water_geometry(context, center, size_x,
        size_y, level);

    GeneratedConfig meta;
    meta.kind = "Water";
    // Связь с текущей траекторией: позволяет снять воду кнопкой
    // «Убрать обвес траектории» окна «Путь»
    meta.trajectory = context.selected_trajectory_name;
    meta.pos_x = center.x;
    meta.pos_y = center.y;
    meta.length = size_x;
    meta.width = size_y;
    meta.level = level;

    // Геометрия воды уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Вода: зеркало " + std::to_string(
        static_cast<int>(size_x)) + "x" + std::to_string(
        static_cast<int>(size_y)) + " м, отметка " +
        std::to_string(static_cast<int>(level)) + " м";

    return true;
}

bool TrackFurniture::generate_crossing(EditorContext& context,
    const std::string& trajectory_name, double coord, double road_length)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    const auto node = create_crossing_geometry(context, trajectory_name,
        coord);

    if (!node)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    GeneratedConfig meta;
    meta.kind = "Crossing";
    meta.trajectory = trajectory_name;
    meta.coord = coord;

    // Геометрия переезда уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    // Дорога у переезда: асфальтовая лента перпендикулярно пути
    // в обе стороны (промт п.11, упрощённо); отдельная запись
    // Kind=Road, удаляется вместе с переездом
    road_length = std::clamp(road_length, 0.0, 1000.0);

    if (road_length > 0.0)
    {
        const auto road_node = create_road_geometry(context,
            trajectory_name, coord, road_length);

        if (road_node)
        {
            GeneratedConfig road_meta;
            road_meta.kind = "Road";
            road_meta.trajectory = trajectory_name;
            road_meta.coord = coord;
            road_meta.length = road_length;

            add_generated_item(context, road_meta, road_node,
                vsg::dmat4());
        }
    }

    context.status = "Переезд: координата " +
        std::to_string(static_cast<int>(coord)) + " м";

    return true;
}

bool TrackFurniture::generate_embankment(EditorContext& context,
    const std::string& trajectory_name, double from, double to,
    double height, double shoulder)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    from = std::max(from, 0.0);

    if (to <= from)
    {
        context.status = "Насыпь: пустой интервал";
        return false;
    }

    // Ограничения габаритов (защита от опечаток в полях)
    height = std::clamp(height, 0.5, 20.0);
    shoulder = std::clamp(shoulder, 3.0, 20.0);

    // Лента-сегменты по 5 м вдоль интервала
    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, from, to, TERRAIN_SEGMENT_STEP);

    if (samples.size() < 2)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    const auto node = create_embankment_geometry(builder, samples,
        height, shoulder);

    GeneratedConfig meta;
    meta.kind = "Embankment";
    meta.trajectory = trajectory_name;
    meta.from = from;
    meta.to = to;
    meta.height = height;
    meta.width = shoulder;

    // Геометрия насыпи уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Насыпь: " + std::to_string(samples.size() - 1) +
        " сегментов, h=" + std::to_string(static_cast<int>(height)) + " м";

    return true;
}

bool TrackFurniture::generate_cutting(EditorContext& context,
    const std::string& trajectory_name, double from, double to,
    double depth, double width)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    from = std::max(from, 0.0);

    if (to <= from)
    {
        context.status = "Выемка: пустой интервал";
        return false;
    }

    // Ограничения габаритов (защита от опечаток в полях)
    depth = std::clamp(depth, 0.5, 20.0);
    width = std::clamp(width, 4.0, 50.0);

    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, from, to, TERRAIN_SEGMENT_STEP);

    if (samples.size() < 2)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    const auto node = create_cutting_geometry(builder, samples,
        depth, width);

    GeneratedConfig meta;
    meta.kind = "Cutting";
    meta.trajectory = trajectory_name;
    meta.from = from;
    meta.to = to;
    meta.height = depth;
    meta.width = width;

    // Геометрия выемки уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Выемка: " + std::to_string(samples.size() - 1) +
        " сегментов, глубина " + std::to_string(static_cast<int>(depth)) +
        " м";

    return true;
}

bool TrackFurniture::generate_ditch(EditorContext& context,
    const std::string& trajectory_name, double from, double to,
    bool right_side, double width, double depth)
{
    if (!furniture_ready(context))
    {
        context.status = "Обвес: топология не загружена";
        return false;
    }

    from = std::max(from, 0.0);

    if (to <= from)
    {
        context.status = "Канава: пустой интервал";
        return false;
    }

    // Ограничения габаритов (защита от опечаток в полях)
    width = std::clamp(width, 0.5, 5.0);
    depth = std::clamp(depth, 0.3, 5.0);

    const std::vector<PathSample> samples = sample_trajectory(context,
        trajectory_name, from, to, TERRAIN_SEGMENT_STEP);

    if (samples.size() < 2)
    {
        context.status = "Обвес: траектория не найдена";
        return false;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    const auto node = create_ditch_geometry(builder, samples, right_side,
        width, depth);

    GeneratedConfig meta;
    meta.kind = "Ditch";
    meta.trajectory = trajectory_name;
    meta.from = from;
    meta.to = to;
    meta.side = right_side ? "right" : "left";
    meta.width = width;
    meta.height = depth;

    // Геометрия канавы уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Канава: " + std::to_string(samples.size() - 1) +
        " сегментов (" + meta.side + ")";

    return true;
}

bool TrackFurniture::generate_proposed_track(EditorContext& context,
    const std::string& name, const std::vector<vsg::dvec3>& control_points)
{
    // Геометрии пути нужна только группа сцены (топология не нужна)
    if (!context.generated_group)
    {
        context.status = "Обвес: маршрут не загружен";
        return false;
    }

    if (name.empty() || control_points.size() < 2)
    {
        context.status = "Новый путь: нужно имя и минимум две точки";
        return false;
    }

    // Сэмплы Catmull-Rom по опорным точкам с шагом 5 м
    const std::vector<vsg::dvec3> points =
        SplineTool::sample_spline_by_step(control_points,
            TRACK_SEGMENT_STEP);

    if (points.size() < 2)
    {
        context.status = "Новый путь: пустая кривая";
        return false;
    }

    std::size_t sleeper_count = 0;

    // У нового пути нет записи профиля - значения по умолчанию (Р65)
    const auto node = build_track_geometry(context, points,
        TrackProfile(), &sleeper_count, nullptr);

    if (!node)
    {
        context.status = "Новый путь: не удалось собрать геометрию";
        return false;
    }

    // Запись сплайна для track-edit.conf (секция ProposedTrack);
    // одноимённая запись заменяется
    context.proposed_tracks.erase(
        std::remove_if(context.proposed_tracks.begin(),
            context.proposed_tracks.end(),
            [&name](const ProposedTrackConfig& track_config) -> bool {
                return track_config.name == name;
            }),
        context.proposed_tracks.end());

    ProposedTrackConfig track_config;
    track_config.name = name;
    track_config.points = control_points;

    context.proposed_tracks.push_back(std::move(track_config));

    GeneratedConfig meta;
    meta.kind = "ProposedTrack";
    meta.trajectory = name;

    // Геометрия пути уже в мировых координатах
    add_generated_item(context, meta, node, vsg::dmat4());

    context.status = "Новый путь \"" + name + "\": " +
        std::to_string(sleeper_count) + " шпал";

    return true;
}

bool TrackFurniture::remove_generated(EditorContext& context,
    const std::string& trajectory_name)
{
    const auto group = context.generated_group;

    if (!group)
    {
        return false;
    }

    // Координаты удаляемых переездов: их дороги (Kind=Road)
    // удаляются вместе с ними по совпадению Traj + Coord
    std::vector<double> crossing_coords;

    for (const GeneratedItem& item : context.generated_items)
    {
        if (item.meta.trajectory == trajectory_name &&
            item.meta.kind == "Crossing")
        {
            crossing_coords.push_back(item.meta.coord);
        }
    }

    const auto is_road_of_removed_crossing =
        [&crossing_coords](const GeneratedConfig& meta) -> bool
    {
        if (meta.kind != "Road")
        {
            return false;
        }

        return std::find(crossing_coords.begin(), crossing_coords.end(),
            meta.coord) != crossing_coords.end();
    };

    bool removed = false;

    // Узлы и записи элементов траектории
    for (auto it = context.generated_items.begin();
         it != context.generated_items.end(); )
    {
        if (it->meta.trajectory != trajectory_name &&
            !is_road_of_removed_crossing(it->meta))
        {
            ++it;
            continue;
        }

        if (it->node)
        {
            auto& children = group->children;

            children.erase(std::find_if(children.begin(), children.end(),
                [&it](const vsg::ref_ptr<vsg::Node>& child) -> bool {
                    return child == it->node;
                }));
        }

        it = context.generated_items.erase(it);
        removed = true;
    }

    // Отложенные записи (топология ещё не загружена)
    context.pending_generated.erase(
        std::remove_if(context.pending_generated.begin(),
            context.pending_generated.end(),
            [&trajectory_name, &is_road_of_removed_crossing]
            (const GeneratedConfig& meta) -> bool {
                return meta.trajectory == trajectory_name ||
                    is_road_of_removed_crossing(meta);
            }),
        context.pending_generated.end());

    if (removed)
    {
        // Перекомпиляция оставшейся части группы (паттерн CompileInfo)
        context.compile_infos.emplace_back(CompileInfo{
            nullptr, vsg::ref_ptr<vsg::Group>(group)});

        context.status = "Обвес траектории удалён: " + trajectory_name;
    }

    return removed;
}

void TrackFurniture::restore_all(EditorContext& context)
{
    if (!context.generated_group)
    {
        return;
    }

    // Записи забираются из staging-а: восстановление либо переводит
    // их в generated_items, либо отбрасывает (нет траектории)
    std::vector<GeneratedConfig> pending;
    pending.swap(context.pending_generated);

    if (pending.empty())
    {
        return;
    }

    vsg::Builder builder;
    setup_builder(builder, context);

    std::size_t restored = 0;

    for (const GeneratedConfig& meta : pending)
    {
        // Платформа: одна запись - лента сегментов
        if (meta.kind == "Platform")
        {
            const std::vector<PathSample> samples = sample_trajectory(
                context, meta.trajectory, 0.0, meta.length,
                PLATFORM_SEGMENT_STEP);

            if (samples.size() < 2)
            {
                continue;
            }

            add_generated_item(context, meta,
                create_platform_geometry(builder, samples, meta.width,
                    meta.height, meta.side != "left"),
                vsg::dmat4());
            ++restored;
            continue;
        }

        // Путь: одна запись - лента рельсов/шпал/балласта.
        // Профиль участка берётся из track_profiles на момент
        // восстановления (нет записи - значения по умолчанию, Р65)
        if (meta.kind == "TrackMesh")
        {
            const auto profile_it = context.track_profiles.find(
                meta.trajectory);

            const TrackProfile profile =
                (profile_it != context.track_profiles.cend())
                    ? profile_it->second
                    : TrackProfile();

            std::size_t sleeper_count = 0;
            bool sleeper_step_increased = false;

            const auto node = create_track_mesh_geometry(context,
                meta.trajectory, meta.from, meta.to, profile,
                sleeper_count, sleeper_step_increased);

            if (node)
            {
                add_generated_item(context, meta, node, vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Деревья: одна запись - лес вдоль траектории (геометрия
        // воспроизводится детерминированным ПСЧ по имени)
        if (meta.kind == "Trees")
        {
            std::size_t tree_count = 0;

            const auto node = create_trees_geometry(context,
                meta.trajectory, meta.side != "left", meta.per_km,
                meta.offset_min, meta.offset_max, tree_count);

            if (node)
            {
                add_generated_item(context, meta, node, vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Вода: одна запись - плоскость в мировой позиции
        if (meta.kind == "Water")
        {
            const vsg::dvec3 center{meta.pos_x, meta.pos_y, 0.0};

            const auto node = create_water_geometry(context, center,
                meta.length, meta.width, meta.level);

            if (node)
            {
                add_generated_item(context, meta, node, vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Переезд: одна запись - настил в координате траектории
        if (meta.kind == "Crossing")
        {
            const auto node = create_crossing_geometry(context,
                meta.trajectory, meta.coord);

            if (node)
            {
                add_generated_item(context, meta, node, vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Дорога у переезда: асфальтовая лента перпендикулярно пути
        if (meta.kind == "Road")
        {
            const auto node = create_road_geometry(context,
                meta.trajectory, meta.coord, meta.length);

            if (node)
            {
                add_generated_item(context, meta, node, vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Насыпь: трапецеидальная земляная лента вдоль интервала
        if (meta.kind == "Embankment")
        {
            const std::vector<PathSample> samples = sample_trajectory(
                context, meta.trajectory, meta.from, meta.to,
                TERRAIN_SEGMENT_STEP);

            if (samples.size() >= 2)
            {
                add_generated_item(context, meta,
                    create_embankment_geometry(builder, samples,
                        meta.height, meta.width),
                    vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Выемка: приподнятые стенки вдоль интервала
        if (meta.kind == "Cutting")
        {
            const std::vector<PathSample> samples = sample_trajectory(
                context, meta.trajectory, meta.from, meta.to,
                TERRAIN_SEGMENT_STEP);

            if (samples.size() >= 2)
            {
                add_generated_item(context, meta,
                    create_cutting_geometry(builder, samples,
                        meta.height, meta.width),
                    vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Канава-кювет: заглублённый бокс вдоль интервала
        if (meta.kind == "Ditch")
        {
            const std::vector<PathSample> samples = sample_trajectory(
                context, meta.trajectory, meta.from, meta.to,
                TERRAIN_SEGMENT_STEP);

            if (samples.size() >= 2)
            {
                add_generated_item(context, meta,
                    create_ditch_geometry(builder, samples,
                        meta.side != "left", meta.width, meta.height),
                    vsg::dmat4());
                ++restored;
            }

            continue;
        }

        // Новый путь: сэмплы Catmull-Rom по точкам из конфига ->
        // общая сборка геометрии пути (профиль по умолчанию, Р65)
        if (meta.kind == "ProposedTrack")
        {
            for (const ProposedTrackConfig& track_config :
                 context.proposed_tracks)
            {
                if (track_config.name != meta.trajectory ||
                    track_config.points.size() < 2)
                {
                    continue;
                }

                const std::vector<vsg::dvec3> points =
                    SplineTool::sample_spline_by_step(
                        track_config.points, TRACK_SEGMENT_STEP);

                const auto node = build_track_geometry(context, points,
                    TrackProfile(), nullptr, nullptr);

                if (node)
                {
                    add_generated_item(context, meta, node, vsg::dmat4());
                    ++restored;
                }

                break;
            }

            continue;
        }

        // Опора КС / километровый столбик: одна запись - один объект
        // в координате вдоль траектории
        const bool is_pole = meta.kind == "CatenaryPole";
        const double offset = is_pole ? POLE_OFFSET : KM_POST_OFFSET;

        const std::vector<PathSample> samples = sample_trajectory(
            context, meta.trajectory, meta.coord, meta.coord, 1.0);

        if (samples.empty())
        {
            continue;
        }

        const PathSample& sample = samples.front();

        const vsg::dvec3 item_pos = sample.position +
            sample.right * offset;

        vsg::dmat4 matrix = vsg::translate(item_pos);
        vsg::ref_ptr<vsg::Node> node;

        if (!meta.label.empty())
        {
            node = create_model_node(context, meta.label);

            if (node)
            {
                matrix = matrix * yaw_to_track(sample);
            }
        }

        if (!node)
        {
            node = is_pole
                ? create_pole_geometry(builder, sample)
                : create_km_post_geometry(builder, sample);
        }

        add_generated_item(context, meta, node, matrix);
        ++restored;
    }

    if (restored > 0)
    {
        Journal::instance()->info(
            QString("Track furniture restored: %1 items")
                .arg(static_cast<uint>(restored)));
    }
}
