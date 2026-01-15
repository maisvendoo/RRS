#include "IntersectionHandler.h"

#include "Gizmo.h"
#include "MouseButton.h"
#include "Outline.h"
#include "Settings.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Array.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/maths/box.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>

IntersectionHandler::IntersectionHandler(
    const settings_t& settings,
    vsg::ref_ptr<vsg::Options> options,
    vsg::ref_ptr<vsg::LookAt> look_at,
    vsg::ref_ptr<vsg::Camera> camera,
    vsg::ref_ptr<vsg::Group> scene_group,
    vsg::ref_ptr<vsg::Group> gui_group,
    vsg::observer_ptr<vsg::Viewer> observer_viewer,
    vsg::ref_ptr<ObjectSelector> object_selector
)
    : settings(settings)
    , options(options)
    , look_at(look_at)
    , camera(camera)
    , scene_group(scene_group)
    , gui_group(gui_group)
    , observer_viewer(observer_viewer)
    , object_selector(object_selector)
{
}

void IntersectionHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled || buttonPress.button != MOUSE_BUTTON_LEFT)
    {
        return;
    }

    const auto intersector = vsg::LineSegmentIntersector::create(*camera, buttonPress.x, buttonPress.y);
    if (!handle_gui_intersections(intersector))
    {
        intersector->intersections.clear();
        handle_scene_intersections(intersector);
    }
}

void IntersectionHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled || buttonRelease.button != MOUSE_BUTTON_LEFT)
    {
        return;
    }

    // active_gizmo_axis = GizmoAxis::NONE;
}

void IntersectionHandler::apply(vsg::MoveEvent& moveEvent)
{
    // if (active_gizmo_axis == GizmoAxis::NONE)
    // {
        // return;
    // }

    // TODO: Replace on real values
    constexpr int screen_width = 2980;
    constexpr int screen_height = 1020;

    const double x = (2.0 * moveEvent.x) / screen_width - 1.0;
    const double y = (2.0 * moveEvent.y) / screen_height - 1.0;

    const vsg::dvec4 ray_clip = {x, y, 0.0, 1.0};

    vsg::dvec4 ray_eye = camera->projectionMatrix->inverse() * ray_clip;
    ray_eye.z = -1.0;
    ray_eye.w = 0.0;

    const vsg::dvec4 ray_world = camera->viewMatrix->inverse() * ray_eye;
    const vsg::dvec3 direction = vsg::normalize(vsg::dvec3(ray_world.x, ray_world.y, ray_world.z));

    double t;
    // switch (active_gizmo_axis)
    // {
    //     case GizmoAxis::X: case GizmoAxis::Y:
    //     {
    //         t = (object_selector->gizmo->translation.z - look_at->eye.z) / direction.z;
    //         break;
    //     }
    //     case GizmoAxis::Z:
    //     {
    //         t = (object_selector->gizmo->translation.y - look_at->eye.y) / direction.y;
    //         break;
    //     }
    //     default:
    //     {
    //         t = 0.0;
    //         break;
    //     }
    // }

    const vsg::dvec3 intersection = look_at->eye + direction * t;

    // switch (active_gizmo_axis)
    // {
    //     case GizmoAxis::X:
    //     {
    //         object_selector->gizmo->translation.x = intersection.x;
    //         break;
    //     }
    //     case GizmoAxis::Y:
    //     {
    //         object_selector->gizmo->translation.y = intersection.y;
    //         break;
    //     }
    //     case GizmoAxis::Z:
    //     {
    //         object_selector->gizmo->translation.z = intersection.z;
    //         break;
    //     }
    //     default:
    //     {
    //         break;
    //     }
    // }
}

void IntersectionHandler::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    // object_selector->gizmo->matrix = vsg::translate(object_selector->gizmo->translation)
        // * vsg::scale(vsg::length(object_selector->gizmo->translation - look_at->eye) * 0.05);
}

vsg::ref_ptr<vsg::MatrixTransform>* IntersectionHandler::get_curr_matrix_transform_ptr()
{
    return &curr_matrix_transform;
}

bool IntersectionHandler::handle_gui_intersections(vsg::ref_ptr<vsg::LineSegmentIntersector> intersector)
{
    gui_group->accept(*intersector);

    if (intersector->intersections.empty())
    {
        return false;
    }

    std::sort(intersector->intersections.begin(), intersector->intersections.end(), [](auto& lhs, auto& rhs) {
        return lhs->ratio < rhs->ratio;
    });

    auto& intersection = *intersector->intersections.front();

    return true;
    // return object_selector->handle_intersection(intersection);
}

void IntersectionHandler::handle_scene_intersections(vsg::ref_ptr<vsg::LineSegmentIntersector> intersector)
{
    scene_group->accept(*intersector);
    if (intersector->intersections.empty())
    {
        return;
    }

    std::sort(intersector->intersections.begin(), intersector->intersections.end(), [](auto& lhs, auto& rhs) {
        return lhs->ratio < rhs->ratio;
    });

    const auto& intersection = *intersector->intersections.front();
    for (const vsg::Node* node : intersection.nodePath)
    {
        vsg::MatrixTransform* const matrix_transform = const_cast<vsg::MatrixTransform*>(node->cast<vsg::MatrixTransform>());
        if (!matrix_transform)
        {
            continue;
        }

        vsg::PagedLOD* const paged_lod = matrix_transform->children[0]->cast<vsg::PagedLOD>();
        if (!paged_lod || !paged_lod->pending)
        {
            continue;
        }

        vsg::ComputeBounds compute_bounds;
        compute_bounds.useNodeBounds = false;
        paged_lod->pending->accept(compute_bounds);

        curr_matrix_transform = matrix_transform;

        vsg::vec3 translationf;
        curr_matrix_transform->getValue("translation", translationf);
        vsg::dvec3 translation(translationf);

        center_offset = (compute_bounds.bounds.min + compute_bounds.bounds.max) * 0.5;

        // object_selector->gizmo->translation = translation + center_offset;

        // object_selector->outline->update(vsg::ref_ptr(paged_lod));
        // object_selector->outline->matrix = curr_matrix_transform->matrix;
    }
}
