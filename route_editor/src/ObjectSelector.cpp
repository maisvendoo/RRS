#include "ObjectSelector.h"

#include "Action.h"
#include "CameraHandler.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "Outline.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "SelectedObjects.h"
#include "SingleSwitch.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    vsg::ref_ptr<CameraHandler> camera_handler,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<SceneGraph> scene_graph,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : settings(settings)
    , keyboard_handler(keyboard_handler)
    , camera_handler(camera_handler)
    , intersection_handler(intersection_handler)
    , scene_graph(scene_graph)
    , observer_viewer(observer_viewer)
{
    assert(keyboard_handler);
    assert(camera_handler);
    assert(intersection_handler);
    assert(scene_graph);
    assert(observer_viewer);

    gizmo = Gizmo::create(settings, camera_handler,
        intersection_handler, selected_objects);

    gizmo_switch = SingleSwitch::create(vsg::MASK_OFF, gizmo);

    scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE}, gizmo_switch);
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    const bool selected_objects_are_empty = selected_objects.empty();

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects_are_empty && gizmo->handle_intersections())
    {
        return;
    }

    const auto intersector = intersection_handler->get_lmb_intersector();
    if (!intersector)
    {
        return;
    }

    scene_graph->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        // If we clicked on empty space without shift
        // while there were selected objects,
        // deselect them all
        if (!selected_objects_are_empty && !keyboard_handler->get_shift_state())
        {
            for (auto it = selected_objects.begin();
                it != selected_objects.end();
                it = deselect_object(*it));
        }

        gizmo_switch->mask = selected_objects.empty()
            ? vsg::MASK_OFF
            : MASK_GUI1 | MASK_CLICKABLE;

        return;
    }

    intersection_handler->sort_intersections(intersections);

    const auto& node_path = intersections.front()->nodePath;
    assert(!node_path.empty());

    for (const vsg::Node* const node : node_path)
    {
        const auto object = vsg::ref_ptr(const_cast<RouteObject*>(
            node->cast<RouteObject>()));

        if (!object)
        {
            continue;
        }

        select_object(object);

        break;
    }

    intersections.clear();

    if (selected_objects.empty())
    {
        gizmo_switch->mask = vsg::MASK_OFF;
    }
    else
    {
        gizmo_switch->mask = MASK_GUI1 | MASK_CLICKABLE;
    }
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    gizmo->apply(moveEvent);
}

void ObjectSelector::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    static bool first_time = true;
    if (first_time && !selected_objects.empty() &&
        keyboard_handler->get_binding_state(ACTION_START_MOVING_OBJECT))
    {
        const auto print_float = [](const char* name, float value) -> void
        {
            std::printf("%s: %10.3f\n", name, value);
        };

        const auto print_vec3 = [](const char* name, vsg::vec3 vec) -> void
        {
            std::printf("%s: %10.3f %10.3f %10.3f\n",
                name, vec.x, vec.y, vec.z);
        };

        first_time = false;

        vsg::vec3 begin_pos = {0.0f, 0.0f, 0.0f};
        for (const auto& object : selected_objects)
        {
            begin_pos += object->get_translation();
        }
        begin_pos /= static_cast<float>(selected_objects.size());
        print_vec3("begin_pos", begin_pos);

        const vsg::vec3 camera_pos = static_cast<vsg::vec3>(
            camera_handler->get_look_at()->eye);
        print_vec3("camera_pos", camera_pos);

        vsg::vec3 camera_front = static_cast<vsg::vec3>(
            camera_handler->get_front());
        camera_front = vsg::normalize(camera_front);
        print_vec3("camera_front", camera_front);

        vsg::vec3 camera_right = static_cast<vsg::vec3>(
            camera_handler->get_right());
        camera_right = vsg::normalize(camera_right);
        print_vec3("camera_right", camera_right);

        vsg::vec3 camera_up = vsg::cross(camera_right, camera_front);
        camera_up = vsg::normalize(camera_up);
        print_vec3("camera_up", camera_up);

        const vsg::vec3 camera_to_object = begin_pos - camera_pos;
        print_vec3("camera_to_object", camera_to_object);

        const float camera_to_object_dist = vsg::length(camera_to_object);
        print_float("camera_to_object_dist", camera_to_object_dist);

        const float cos_a = vsg::dot(camera_front,
            vsg::normalize(camera_to_object));
        print_float("cos_a", cos_a);

        const float norm_length = camera_to_object_dist * cos_a;
        print_float("norm_length", norm_length);

        const vsg::vec3 norm = camera_front * norm_length;
        print_vec3("norm", norm);

        const float half_fov = static_cast<float>(vsg::radians(
            camera_handler->get_perspective()->fieldOfViewY / 2.0));
        print_float("half_fov", half_fov);

        const float cos_b = std::cos(half_fov);
        print_float("cos_b", cos_b);

        const float dist = norm_length / cos_b;
        print_float("dist", dist);

        vsg::vec3 botton_left_dir = camera_front * vsg::rotate(
            -half_fov, camera_up) * vsg::rotate(half_fov, camera_right);
        botton_left_dir = vsg::normalize(botton_left_dir);
        print_vec3("bottom_left_dir", botton_left_dir);

        vsg::vec3 bottom_right_dir = camera_front * vsg::rotate(
            half_fov, camera_up) * vsg::rotate(half_fov, camera_right);
        bottom_right_dir = vsg::normalize(bottom_right_dir);
        print_vec3("bottom_right_dir", bottom_right_dir);

        vsg::vec3 top_left_dir = camera_front * vsg::rotate(
            -half_fov, camera_up) * vsg::rotate(-half_fov, camera_right);
        top_left_dir = vsg::normalize(top_left_dir);
        print_vec3("top_left_dir", top_left_dir);

        vsg::vec3 top_right_dir = camera_front * vsg::rotate(
            half_fov, camera_up) * vsg::rotate(-half_fov, camera_right);
        top_right_dir = vsg::normalize(top_right_dir);
        print_vec3("top_right_dir", top_right_dir);
    }

    gizmo->update_position();
    gizmo->update_scale();
}

const SelectedObjects& ObjectSelector::get_selected_objects() const
{
    return selected_objects;
}

void ObjectSelector::select_object(vsg::ref_ptr<RouteObject> object)
{
    const auto found_it = std::find(selected_objects.cbegin(),
        selected_objects.cend(), object);

    const bool clicked_on_selected_object = found_it != selected_objects.cend();

    const auto select_object_inner = [&]() -> void
    {
        const auto viewer = observer_viewer.ref_ptr();
        const auto compile_manager = viewer->compileManager;

        const auto outline = Outline::create(settings, object);

        const auto compile_result = compile_manager->compile(outline);

        object->get_switch_group()->addChild(vsg::Mask{MASK_GUI2}, outline);
        object->outline = outline;
        object->update_bounds();

        vsg::updateViewer(*viewer, compile_result);

        selected_objects.emplace_back(object);
    };

    if (keyboard_handler->get_shift_state())
    {
        if (clicked_on_selected_object)
        {
            deselect_object(object);
        }
        else
        {
            select_object_inner();
        }
    }
    else
    {
        if (selected_objects.empty())
        {
            select_object_inner();
        }
        else if (clicked_on_selected_object)
        {
            if (selected_objects.size() == 1)
            {
                deselect_object(object);
            }
            else
            {
                for (auto it = selected_objects.begin();
                    it != selected_objects.end();)
                {
                    if (*it == object)
                    {
                        ++it;
                    }
                    else
                    {
                        it = deselect_object(*it);
                    }
                }
            }
        }
        else
        {
            for (auto it = selected_objects.begin();
                it != selected_objects.end();
                it = deselect_object(*it));

            select_object_inner();
        }
    }
}

SelectedObjectsIterator ObjectSelector::deselect_object(
    vsg::ref_ptr<RouteObject> object
)
{
    assert(object);

    const auto switch_group = object->get_switch_group();
    auto& switch_group_children = switch_group->children;

    const auto outline = object->outline;

    for (auto it = switch_group_children.begin();
        it != switch_group_children.end(); ++it)
    {
        if (it->node == outline)
        {
            switch_group_children.erase(it);

            break;
        }
    }

    return selected_objects.erase(std::find(selected_objects.cbegin(),
        selected_objects.cend(), object));
}
