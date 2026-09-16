#include "editor/Validator.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"
#include "editor/states/EditorState.h"

#include <profile-point.h>
#include <topology-defines.h>
#include <topology.h>
#include <trajectory.h>
#include <vec3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

/// Порог близости пары объектов, м (вероятный дубль)
static constexpr double DUPLICATE_DISTANCE = 0.5;

/// Лимит объектов для O(N^2) проверки дублей
static constexpr std::size_t DUPLICATE_CHECK_LIMIT = 5000;

/// Максимум находок-дублей в списке (чтобы не заливать окно)
static constexpr std::size_t MAX_DUPLICATE_ISSUES = 50;

/// Радиус цилиндра габарита вокруг оси пути, м (проверка
/// свободности подходов к пути, промт п.25)
static constexpr double CLEARANCE_RADIUS = 2.5;

/// Шаг сэмплирования оси пути, м
static constexpr double CLEARANCE_SAMPLING_STEP = 10.0;

/// Максимум точек сэмплирования на одну траекторию
static constexpr std::size_t CLEARANCE_MAX_SAMPLES = 4000;

/// Максимум находок проверки габаритов
static constexpr std::size_t MAX_CLEARANCE_ISSUES = 50;

/// Все компоненты вектора конечны
static bool is_finite(const vsg::dvec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) &&
        std::isfinite(value.z);
}

std::vector<ValidationIssue> Validator::validate(EditorContext& context)
{
    std::vector<ValidationIssue> issues;

    // (а) Объекты route1.map с label, отсутствующим в objects.ref
    for (const auto& [label, transforms] : context.route_map)
    {
        if (context.objects_ref.find(label) != context.objects_ref.end())
        {
            continue;
        }

        ValidationIssue issue;
        issue.type = ValidationType::Error;

        issue.text = "Объект \"" + label + "\" из route1.map отсутствует "
            "в objects.ref (" + std::to_string(transforms.size()) + " шт.)";

        if (!transforms.empty() && is_finite(transforms.front().translation))
        {
            issue.position = transforms.front().translation;
            issue.has_position = true;
        }

        issues.emplace_back(std::move(issue));
    }

    // (б) NaN/Inf в трансформациях объектов route1.map
    for (const auto& [label, transforms] : context.route_map)
    {
        for (std::size_t i = 0; i < transforms.size(); ++i)
        {
            const RouteMapTransformation& transform = transforms[i];

            if (is_finite(transform.translation) &&
                is_finite(transform.rotation_deg))
            {
                continue;
            }

            ValidationIssue issue;
            issue.type = ValidationType::Error;

            char index_text[32];
            std::snprintf(index_text, sizeof(index_text), "%zu", i + 1);

            issue.text = "Объект \"" + label + "\" (строка " + index_text +
                " route1.map): NaN/Inf в трансформации";

            if (is_finite(transform.translation))
            {
                issue.position = transform.translation;
                issue.has_position = true;
            }

            issues.emplace_back(std::move(issue));
        }
    }

    // (б) NaN/Inf в загруженных объектах сцены
    {
        std::lock_guard<std::mutex> lock_guard(context.static_objects_mutex);

        for (const auto& object : context.static_objects)
        {
            const bool translation_finite =
                is_finite(object->get_translation());
            const bool rotation_finite = is_finite(object->get_rotation_deg());
            const bool scale_finite = is_finite(object->get_scale());

            if (translation_finite && rotation_finite && scale_finite)
            {
                continue;
            }

            ValidationIssue issue;
            issue.type = ValidationType::Error;

            std::string part;

            if (!translation_finite)
            {
                part += " translation";
            }

            if (!rotation_finite)
            {
                part += " rotation";
            }

            if (!scale_finite)
            {
                part += " scale";
            }

            issue.text = "Объект \"" + object->label + "\": NaN/Inf в" +
                part;

            if (translation_finite)
            {
                issue.position = object->get_translation();
                issue.has_position = true;
            }

            issues.emplace_back(std::move(issue));
        }
    }

    // (в) Пары объектов ближе 0.5 м (вероятные дубликаты)
    {
        std::lock_guard<std::mutex> lock_guard(context.static_objects_mutex);

        if (context.static_objects.size() > DUPLICATE_CHECK_LIMIT)
        {
            ValidationIssue issue;
            issue.type = ValidationType::Warning;

            issue.text = "Объектов больше " +
                std::to_string(DUPLICATE_CHECK_LIMIT) +
                ", проверка близких пар пропущена";

            issues.emplace_back(std::move(issue));
        }
        else
        {
            std::vector<vsg::dvec3> centers;
            std::vector<const std::string*> labels;
            centers.reserve(context.static_objects.size());
            labels.reserve(context.static_objects.size());

            for (const auto& object : context.static_objects)
            {
                const vsg::dvec3& translation = object->get_translation();

                if (!is_finite(translation))
                {
                    continue;
                }

                centers.push_back(translation);
                labels.push_back(&object->label);
            }

            std::size_t duplicate_issues = 0;
            std::size_t duplicate_pairs = 0;

            for (std::size_t i = 0; i < centers.size(); ++i)
            {
                for (std::size_t j = i + 1; j < centers.size(); ++j)
                {
                    if (vsg::length(centers[j] - centers[i]) >=
                        DUPLICATE_DISTANCE)
                    {
                        continue;
                    }

                    ++duplicate_pairs;

                    if (duplicate_issues >= MAX_DUPLICATE_ISSUES)
                    {
                        continue;
                    }

                    ++duplicate_issues;

                    ValidationIssue issue;
                    issue.type = ValidationType::Warning;

                    char distance_text[32];
                    std::snprintf(distance_text, sizeof(distance_text),
                        "%.2f", vsg::length(centers[j] - centers[i]));

                    issue.text = "Объекты \"" + *labels[i] + "\" и \"" +
                        *labels[j] + "\" почти совпадают (" +
                        distance_text + " м) - возможен дубль";

                    issue.position = centers[i];
                    issue.has_position = true;

                    issues.emplace_back(std::move(issue));
                }
            }

            if (duplicate_pairs > duplicate_issues)
            {
                ValidationIssue issue;
                issue.type = ValidationType::Warning;

                issue.text = "...и ещё " +
                    std::to_string(duplicate_pairs - duplicate_issues) +
                    " близких пар (показаны первые " +
                    std::to_string(duplicate_issues) + ")";

                issues.emplace_back(std::move(issue));
            }
        }
    }

    // (г) Станции без позиции (нули или NaN)
    for (const auto& [label, translation] : context.stations_conf)
    {
        const bool finite = is_finite(translation);

        const bool zero = finite &&
            translation.x == 0.0 && translation.y == 0.0 &&
            translation.z == 0.0;

        if (finite && !zero)
        {
            continue;
        }

        ValidationIssue issue;
        issue.type = ValidationType::Warning;

        issue.text = "Станция \"" + label +
            "\" без позиции (0, 0, 0 или NaN)";

        if (finite)
        {
            issue.position = translation;
            issue.has_position = true;
        }

        issues.emplace_back(std::move(issue));
    }

    // (д) Топология не загружена
    if (context.state == EditorState::EDIT_ROUTE &&
        !context.topology_loaded.load())
    {
        ValidationIssue issue;
        issue.type = ValidationType::Warning;

        issue.text = "Топология маршрута не загружена "
            "(проверьте topology/topology.xml)";

        issues.emplace_back(std::move(issue));
    }

    return issues;
}

std::vector<ValidationIssue> Validator::check_clearance(EditorContext& context)
{
    std::vector<ValidationIssue> issues;

    if (!context.topology_loaded.load())
    {
        ValidationIssue issue;
        issue.type = ValidationType::Warning;
        issue.text = "Проверка габаритов: топология не загружена";

        issues.emplace_back(std::move(issue));
        return issues;
    }

    // Копия данных объектов: метка, позиция, план-габарит
    // (снимок под мьютексом, чтобы не держать его при обходе топологии)
    struct ObjectInfo
    {
        const std::string* label = nullptr;
        vsg::dvec3 position = {0.0, 0.0, 0.0};
        double size_xy = 0.0;
    };

    std::vector<ObjectInfo> object_infos;

    {
        std::lock_guard<std::mutex> lock_guard(context.static_objects_mutex);

        object_infos.reserve(context.static_objects.size());

        for (const auto& object : context.static_objects)
        {
            const vsg::dvec3& translation = object->get_translation();

            if (!is_finite(translation))
            {
                continue;
            }

            // План-габарит: полудиагональ bbox в плане XY
            double size_xy = 0.0;

            const vsg::dbox& bounds = object->get_bounds();

            if (bounds.valid())
            {
                const vsg::dvec3 half = (bounds.max - bounds.min) * 0.5;
                size_xy = std::hypot(half.x, half.y);
            }

            object_infos.push_back(ObjectInfo{&object->label, translation,
                size_xy});
        }
    }

    if (object_infos.empty())
    {
        return issues;
    }

    // Оси всех траекторий, сэмплированные каждые 10 м
    struct TrajAxis
    {
        std::string name;
        std::vector<vsg::dvec3> points;
    };

    std::vector<TrajAxis> axes;

    {
        std::lock_guard<std::mutex> lock_guard(context.topology_mutex);

        if (!context.topology)
        {
            return issues;
        }

        const traj_list_t* const traj_list =
            context.topology->getTrajectoriesList();

        for (const Trajectory* trajectory : *traj_list)
        {
            const double length = trajectory->getLength();

            if (length < 1.0e-6)
            {
                continue;
            }

            TrajAxis axis;
            axis.name = trajectory->getName().toStdString();

            const std::size_t count =
                std::min<std::size_t>(CLEARANCE_MAX_SAMPLES - 1,
                    static_cast<std::size_t>(
                        length / CLEARANCE_SAMPLING_STEP)) + 1;

            axis.points.reserve(count);

            for (std::size_t i = 0; i < count; ++i)
            {
                const profile_point_t point = trajectory->getPosition(
                    length * static_cast<double>(i) /
                        static_cast<double>(count - 1),
                    1);

                axis.points.push_back(vsg::dvec3{point.position.x,
                    point.position.y, point.position.z});
            }

            axes.push_back(std::move(axis));
        }
    }

    if (axes.empty())
    {
        return issues;
    }

    // Расстояние в плане от объекта до оси пути < 2.5 м + план-габарит
    std::size_t found = 0;

    for (const ObjectInfo& info : object_infos)
    {
        for (const TrajAxis& axis : axes)
        {
            double best_sq = std::numeric_limits<double>::max();

            for (const vsg::dvec3& point : axis.points)
            {
                const double dx = point.x - info.position.x;
                const double dy = point.y - info.position.y;

                best_sq = std::min(best_sq, dx * dx + dy * dy);
            }

            const double distance = std::sqrt(best_sq);

            if (distance >= CLEARANCE_RADIUS + info.size_xy)
            {
                continue;
            }

            ++found;

            if (issues.size() < MAX_CLEARANCE_ISSUES)
            {
                ValidationIssue issue;
                issue.type = ValidationType::Warning;

                char distance_text[32];
                std::snprintf(distance_text, sizeof(distance_text),
                    "%.2f", distance);

                issue.text = "Объект \"" + *info.label +
                    "\" близко к пути \"" + axis.name + "\" (" +
                    distance_text + " м)";

                issue.position = info.position;
                issue.has_position = true;

                issues.emplace_back(std::move(issue));
            }

            break;
        }

        if (issues.size() >= MAX_CLEARANCE_ISSUES)
        {
            break;
        }
    }

    if (found > issues.size())
    {
        ValidationIssue issue;
        issue.type = ValidationType::Warning;

        issue.text = "...и ещё " + std::to_string(found - issues.size()) +
            " объектов близко к путям (показаны первые " +
            std::to_string(issues.size()) + ")";

        issues.emplace_back(std::move(issue));
    }

    return issues;
}
