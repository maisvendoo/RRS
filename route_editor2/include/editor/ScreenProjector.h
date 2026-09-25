#ifndef EDITOR_SCREEN_PROJECTOR_H
#define EDITOR_SCREEN_PROJECTOR_H

#include "editor/Camera.h"
#include "editor/EditorContext.h"

#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/state/ViewportState.h>

#include <cmath>

namespace editor2
{

/**
 * @brief Спроецировать мировую точку в координаты окна (пиксели,
 *        ось Y направлена вниз, как в ImGui и событиях мыши vsg).
 *
 * Матрица proj * view собирается вручную из параметров камеры
 * (Camera отдаёт perspective и look_at) по аналогии с gluPerspective.
 *
 * @return false, если точка находится за камерой или вне отсечения.
 */
inline bool project_world_to_screen(const EditorContext& context,
                                    const vsg::dvec3& world,
                                    vsg::dvec2& screen)
{
    const vsg::ref_ptr<Camera> camera = context.camera;

    if (!camera || !camera->get_perspective() || !camera->get_look_at() ||
        !camera->viewportState)
    {
        return false;
    }

    const vsg::Perspective* const perspective =
        camera->get_perspective().get();

    const vsg::LookAt* const look_at = camera->get_look_at().get();

    // Перспективная проекция из параметров камеры (углы - в градусах)
    const double fovy_rad = vsg::radians(perspective->fieldOfViewY);
    const double tan_half_fovy = std::tan(fovy_rad * 0.5);
    const double aspect = perspective->aspectRatio;
    const double z_near = perspective->nearDistance;
    const double z_far = perspective->farDistance;

    const vsg::dmat4 projection(
        vsg::dvec4{1.0 / (aspect * tan_half_fovy), 0.0, 0.0, 0.0},
        vsg::dvec4{0.0, 1.0 / tan_half_fovy, 0.0, 0.0},
        vsg::dvec4{0.0, 0.0, -(z_far + z_near) / (z_far - z_near), -1.0},
        vsg::dvec4{0.0, 0.0,
            -(2.0 * z_far * z_near) / (z_far - z_near), 0.0});

    const vsg::dmat4 view = vsg::lookAt(look_at->eye, look_at->center,
        look_at->up);

    const vsg::dvec4 clip = projection * view * vsg::dvec4{world, 1.0};

    // Точка за камерой
    if (clip.w <= 0.0)
    {
        return false;
    }

    const vsg::dvec3 ndc = vsg::dvec3{clip.x, clip.y, clip.z} / clip.w;

    // Точка за дальней или перед ближней плоскостью отсечения
    if (ndc.z < -1.0 || ndc.z > 1.0)
    {
        return false;
    }

    if (camera->viewportState->viewports.empty())
    {
        return false;
    }

    const VkViewport& viewport = camera->viewportState->viewports.front();

    screen.x = static_cast<double>(viewport.x) +
        (ndc.x + 1.0) * 0.5 * static_cast<double>(viewport.width);

    screen.y = static_cast<double>(viewport.y) +
        (1.0 - (ndc.y + 1.0) * 0.5) * static_cast<double>(viewport.height);

    return true;
}

} // namespace editor2

#endif // EDITOR_SCREEN_PROJECTOR_H
