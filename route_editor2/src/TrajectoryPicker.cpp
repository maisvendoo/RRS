#include "editor/TrajectoryPicker.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/MouseButton.h"
#include "editor/Route.h"
#include "editor/states/EditorState.h"

#include <topology-defines.h>
#include <topology.h>
#include <trajectory.h>

#include <profile-point.h>
#include <vec3.h>

#include <vsgImGui/imgui.h>

#include <vsg/app/Camera.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/PointerEvent.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <string>

/// Шаг выборки точек траектории для проверки пересечения, м
static constexpr double PICK_SAMPLING_STEP = 10.0;

/// Минимальный радиус захвата траектории курсором, м
static constexpr double PICK_MIN_RADIUS = 2.0;

/// Радиус захвата - доля дистанции до траектории (захват издалека шире)
static constexpr double PICK_RELATIVE_RADIUS = 0.01;

static vsg::dvec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::dvec3{vec.x, vec.y, vec.z};
}

/// Расстояние между отрезками p1q1 (луч курсора) и p2q2 (кусок полилинии);
/// fraction - положение ближайшей точки на луче (0..1).
/// (Классический алгоритм ближайших точек отрезков, Эриксон)
static double segments_distance(const vsg::dvec3& p1, const vsg::dvec3& q1,
    const vsg::dvec3& p2, const vsg::dvec3& q2, double* fraction = nullptr)
{
    const vsg::dvec3 d1 = q1 - p1;
    const vsg::dvec3 d2 = q2 - p2;
    const vsg::dvec3 r = p1 - p2;

    const double a = vsg::dot(d1, d1);
    const double e = vsg::dot(d2, d2);
    const double f = vsg::dot(d2, r);

    constexpr double eps = 1.0e-12;

    double s = 0.0;
    double t = 0.0;

    if (a <= eps && e <= eps)
    {
        if (fraction != nullptr)
        {
            *fraction = 0.0;
        }

        return vsg::length(p1 - p2);
    }

    if (a <= eps)
    {
        // Первый отрезок вырожден в точку
        s = 0.0;
        t = std::clamp(f / e, 0.0, 1.0);
    }
    else
    {
        const double c = vsg::dot(d1, r);

        if (e <= eps)
        {
            // Второй отрезок вырожден в точку
            t = 0.0;
            s = std::clamp(-c / a, 0.0, 1.0);
        }
        else
        {
            const double b = vsg::dot(d1, d2);
            const double denom = a * e - b * b;

            if (denom > eps)
            {
                s = std::clamp((b * f - c * e) / denom, 0.0, 1.0);
            }
            else
            {
                s = 0.0;
            }

            t = (b * s + f) / e;

            if (t < 0.0)
            {
                t = 0.0;
                s = std::clamp(-c / a, 0.0, 1.0);
            }
            else if (t > 1.0)
            {
                t = 1.0;
                s = std::clamp((b - c) / a, 0.0, 1.0);
            }
        }
    }

    if (fraction != nullptr)
    {
        *fraction = s;
    }

    const vsg::dvec3 closest1 = p1 + d1 * s;
    const vsg::dvec3 closest2 = p2 + d2 * t;

    return vsg::length(closest1 - closest2);
}

namespace
{

/// Сегмент клика в мировых координатах (повторяет математику
/// vsg::LineSegmentIntersector, но переводит результат в мир)
bool computeWorldSegment(const vsg::Camera& camera, int x, int y,
                         vsg::dvec3& start, vsg::dvec3& end)
{
    if (!camera.projectionMatrix || !camera.viewMatrix)
    {
        return false;
    }

    const auto viewport = camera.getViewport();

    vsg::vec2 ndc(0.0f, 0.0f);
    if ((viewport.width > 0) && (viewport.height > 0))
    {
        ndc.set((static_cast<float>(x) - viewport.x) / viewport.width,
                (static_cast<float>(y) - viewport.y) / viewport.height);
    }

    const auto projectionMatrix = camera.projectionMatrix->transform();
    const auto viewMatrix = camera.viewMatrix->transform();

    const bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    const vsg::dvec3 ndc_near(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                              reverse_depth ? viewport.maxDepth : viewport.minDepth);
    const vsg::dvec3 ndc_far(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                             reverse_depth ? viewport.minDepth : viewport.maxDepth);

    const auto inv_projectionMatrix = vsg::inverse(projectionMatrix);
    const vsg::dvec3 eye_near = inv_projectionMatrix * ndc_near;
    const vsg::dvec3 eye_far = inv_projectionMatrix * ndc_far;

    const vsg::dmat4 eye_to_world = vsg::inverse(viewMatrix);
    start = eye_to_world * eye_near;
    end = eye_to_world * eye_far;

    return true;
}

} // namespace

TrajectoryPicker::TrajectoryPicker(EditorContext& context)
    : context_(context)
{
}

void TrajectoryPicker::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    if (buttonPress.button != editor2::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    // Клик по окну ImGui не должен выбирать траектории
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    if (!context_.trajectory_mode || !context_.route)
    {
        return;
    }

    // Мировой луч клика: та же математика, что в
    // vsg::LineSegmentIntersector(camera, x, y), но с переводом в мир
    vsg::dvec3 ray_start;
    vsg::dvec3 ray_end;
    if (!computeWorldSegment(*context_.camera, buttonPress.x, buttonPress.y,
                             ray_start, ray_end))
    {
        return;
    }

    Trajectory* picked = nullptr;

    {
        std::lock_guard<std::mutex> lock_guard(context_.topology_mutex);

        if (!context_.topology)
        {
            return;
        }

        picked = pick_closest_trajectory(ray_start, ray_end);
    }

    if (picked != nullptr)
    {
        context_.route->select_trajectory(picked->getName().toStdString());

        context_.status = "Selected trajectory: " +
            context_.selected_trajectory_name;
    }
    else
    {
        // Клик мимо траекторий - снять выделение
        context_.route->select_trajectory("");
    }
}

Trajectory* TrajectoryPicker::pick_closest_trajectory(
    const vsg::dvec3& ray_begin,
    const vsg::dvec3& ray_end) const
{
    // Вызывается под topology_mutex
    const traj_list_t* const traj_list =
        context_.topology->getTrajectoriesList();

    const vsg::dvec3 ray_vector = ray_end - ray_begin;
    const double ray_length = vsg::length(ray_vector);

    if (ray_length < 1.0e-9)
    {
        return nullptr;
    }

    const vsg::dvec3 ray_dir_norm = ray_vector / ray_length;

    Trajectory* best_trajectory = nullptr;
    double best_distance = std::numeric_limits<double>::max();
    double best_ray_fraction = 0.0;

    // Быстрый префильтр по AABB траектории (Track::begin/end points):
    // отсекаем далёкие пути до дорогого семплирования оси
    auto trajectory_bbox = [](Trajectory* t, vsg::dvec3& lo, vsg::dvec3& hi)
    {
        const auto& tracks = t->getTracks();
        if (tracks.empty())
        {
            return false;
        }

        lo = vsg::dvec3(1e18, 1e18, 1e18);
        hi = vsg::dvec3(-1e18, -1e18, -1e18);

        for (const track_t& track : tracks)
        {
            const dvec3 pts[2] = {track.begin_point, track.end_point};

            for (const dvec3& p : pts)
            {
                lo.x = std::min(lo.x, p.x);
                lo.y = std::min(lo.y, p.y);
                lo.z = std::min(lo.z, p.z);
                hi.x = std::max(hi.x, p.x);
                hi.y = std::max(hi.y, p.y);
                hi.z = std::max(hi.z, p.z);
            }
        }

        return true;
    };

    // Расстояние точка-луч, расстояние от AABB до луча (по вершинам,
    // грубая нижняя оценка: пропускаем траекторию, если ЛУБОВАЯ точка
    // её бокса дальше текущего лучшего кандидата + запас 50 м)
    for (Trajectory* trajectory : *traj_list)
    {
        const double length = trajectory->getLength();

        if (length < 1.0e-9)
        {
            continue;
        }

        if (best_distance < std::numeric_limits<double>::max())
        {
            vsg::dvec3 lo;
            vsg::dvec3 hi;

            if (trajectory_bbox(trajectory, lo, hi))
            {
                double box_min_dist = 1e18;

                for (int cx = 0; cx < 2; ++cx)
                {
                    for (int cy = 0; cy < 2; ++cy)
                    {
                        for (int cz = 0; cz < 2; ++cz)
                        {
                            const vsg::dvec3 corner(cx ? hi.x : lo.x,
                                                    cy ? hi.y : lo.y,
                                                    cz ? hi.z : lo.z);
                            const vsg::dvec3 v = corner - ray_begin;
                            const double t = vsg::dot(v, ray_dir_norm);

                            if (t < 0.0)
                            {
                                box_min_dist = 0.0;
                            }
                            else
                            {
                                const double d = vsg::length(v - ray_dir_norm * t);
                                box_min_dist = std::min(box_min_dist, d);
                            }
                        }
                    }
                }

                if (box_min_dist > best_distance + 50.0)
                {
                    continue;
                }
            }
        }

        // Точки полилинии каждые ~10 м (минимум начало и конец)
        const std::size_t samples = std::max<std::size_t>(2,
            static_cast<std::size_t>(
                std::ceil(length / PICK_SAMPLING_STEP)) + 1);

        vsg::dvec3 prev_point = to_vsg_vec3(
            trajectory->getPosition(0.0, 1).position);

        for (std::size_t i = 1; i < samples; ++i)
        {
            const double coord = length * static_cast<double>(i) /
                static_cast<double>(samples - 1);

            const vsg::dvec3 curr_point = to_vsg_vec3(
                trajectory->getPosition(coord, 1).position);

            double ray_fraction = 0.0;

            const double distance = segments_distance(ray_begin, ray_end,
                prev_point, curr_point, &ray_fraction);

            if (distance < best_distance)
            {
                best_distance = distance;
                best_ray_fraction = ray_fraction;
                best_trajectory = trajectory;
            }

            prev_point = curr_point;
        }
    }

    if (best_trajectory == nullptr)
    {
        return nullptr;
    }

    // Порог захвата растёт с дистанцией до траектории
    const double pick_radius = std::max(PICK_MIN_RADIUS,
        best_ray_fraction * ray_length * PICK_RELATIVE_RADIUS);

    if (best_distance > pick_radius)
    {
        return nullptr;
    }

    return best_trajectory;
}
