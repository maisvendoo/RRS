#include "ObjectSelector.h"

#include "Action.h"
#include "CameraHandler.h"
#include "CommandList.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "MouseHandler.h"
#include "MoveObjectsCommand.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "SelectObjectsCommand.h"
#include "SingleSwitch.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>

#include <cassert>
#include <utility>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    CommandList& commands,
    vsg::ref_ptr<MouseHandler> mouse_handler,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    vsg::ref_ptr<CameraHandler> camera_handler,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<SceneGraph> scene_graph,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : settings(settings)
    , commands(commands)
    , mouse_handler(mouse_handler)
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

    gizmo = Gizmo::create(settings, commands, camera_handler,
        intersection_handler, RouteObject::get_selected_objects());

    front_plane_switch = SingleSwitch::create(
        vsg::Mask{MASK_CLICKABLE}, nullptr);

    scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE}, gizmo);
    scene_graph->addChild(vsg::Mask{MASK_CLICKABLE}, front_plane_switch);
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    switch (state)
    {
        case State::INITIAL:
        {
            break;
        }
        case State::KEYBOARD_GRAB:
        {
            if (mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_move();
            }
            else if (mouse_handler->get_is_rmb_pressed())
            {
                cancel_keyboard_move();
            }

            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            if (mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_rotate();
            }
            else if (mouse_handler->get_is_rmb_pressed())
            {
                cancel_keyboard_rotate();
            }

            return;
        }
        case State::KEYBOARD_SCALE:
        {
            if (mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_scale();
            }
            else if (mouse_handler->get_is_rmb_pressed())
            {
                cancel_keyboard_scale();
            }

            return;
        }
        default:
        {
            break;
        }
    }

    const auto& selected_objects = RouteObject::get_selected_objects();

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects.empty() && gizmo->handle_intersections())
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
        if (!selected_objects.empty() &&
            !keyboard_handler->get_any_shift_state())
        {
            commands.push(new SelectObjectsCommand({}, selected_objects), true);
        }

        return;
    }

    const auto intersection = intersection_handler->get_closest_intersection(
        intersector);

    if (!intersection)
    {
        intersections.clear();
        return;
    }

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if (RouteObject* object = const_cast<RouteObject*>(
            node->cast<RouteObject>()))
        {
            select_object(object);
            break;
        }
    }

    intersections.clear();
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    gizmo->apply(moveEvent);

    const auto front_plane = front_plane_switch->node;
    if (!front_plane)
    {
        return;
    }

    const auto intersector = intersection_handler->apply_(moveEvent);

    front_plane->accept(*intersector);

    const auto intersection =
        intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    const auto world_intersection = static_cast<vsg::vec3>(
        intersection->worldIntersection);

    intersector->intersections.clear();

    switch (state)
    {
        case State::INITIAL:
        {
            return;
        }
        case State::KEYBOARD_GRAB:
        {
            // const auto translation = world_intersection - prev_intersect_pos;
            // prev_intersect_pos = world_intersection;
            // total_translation += translation;

            // for (const auto& object : RouteObject::get_selected_objects())
            // {
            //     object->move(translation);
            // }

            vsg::vec3 center = {0.0f, 0.0f, 0.0f};
            const auto& selected_objects = RouteObject::get_selected_objects();
            for (const auto& object : selected_objects)
            {
                center += object->get_translation();
            }
            center /= static_cast<float>(selected_objects.size());

            if (prev_intersect_pos == begin_intersect_pos)
            {
                prev_intersect_pos = world_intersection;
            }

            const auto dot1 = vsg::dot(
                vsg::normalize(world_intersection - begin_intersect_pos),
                vsg::normalize(front_plane_up)
            );

            const auto dot2 = vsg::dot(
                vsg::normalize(prev_intersect_pos - begin_intersect_pos),
                vsg::normalize(front_plane_up)
            );

            if (std::abs(dot1) < 0.99f && std::abs(dot2) < 0.99f)
            {
                const auto acos1 = std::acos(dot1);
                const auto acos2 = std::acos(dot2);
                const auto rotation_deg = front * vsg::degrees(acos1 - acos2);

                prev_intersect_pos = world_intersection;

                for (const auto& object : selected_objects)
                {
                    object->rotate_around_pivot(center, rotation_deg, object->matrix);
                }
            }


            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            return;
        }
        case State::KEYBOARD_SCALE:
        {
            return;
        }
        default:
        {
            return;
        }
    }
}

void ObjectSelector::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    const auto& selected_objects = RouteObject::get_selected_objects();

    if (state != State::KEYBOARD_GRAB && !selected_objects.empty() &&
        keyboard_handler->get_binding_state(ACTION_MOVE_OBJECTS))
    {
        vsg::vec3 begin_pos = {0.0f, 0.0f, 0.0f};
        for (const auto& object : selected_objects)
        {
            begin_pos += object->get_translation();
        }
        begin_pos /= static_cast<float>(selected_objects.size());

        const auto front_plane = camera_handler->create_front_plane(begin_pos,
            &front_plane_up);

        front = camera_handler->get_front();

        const auto intersector = intersection_handler->apply_(
            mouse_handler->get_pos());

        if (!intersector)
        {
            return;
        }

        front_plane->accept(*intersector);

        const auto intersection =
            intersection_handler->get_closest_intersection(intersector);

        if (!intersection)
        {
            return;
        }

        begin_intersect_pos = static_cast<vsg::vec3>(
            intersection->worldIntersection);

        prev_intersect_pos = begin_intersect_pos;
        total_translation = {0.0f, 0.0f, 0.0f};

        const auto viewer = observer_viewer.ref_ptr();
        const auto compile_manager = viewer->compileManager;
        const auto compile_result = compile_manager->compile(front_plane);

        state = State::KEYBOARD_GRAB;
        front_plane_switch->node = front_plane;

        vsg::updateViewer(*viewer, compile_result);
    }
}

void ObjectSelector::select_object(RouteObject* object)
{
    if (keyboard_handler->get_any_shift_state())
    {
        if (object->get_is_selected())
        {
            commands.push(new SelectObjectsCommand({}, {object}), true);
        }
        else
        {
            commands.push(new SelectObjectsCommand({object}, {}), true);
        }
    }
    else
    {
        const auto& selected_objects = RouteObject::get_selected_objects();

        if (selected_objects.empty())
        {
            commands.push(new SelectObjectsCommand({object}, {}), true);
        }
        else if (object->get_is_selected())
        {
            RouteObjects objects_to_deselect;

            if (selected_objects.size() == 1)
            {
                objects_to_deselect.emplace_back(object);
            }
            else
            {
                for (const auto& selected_object : selected_objects)
                {
                    if (selected_object != object)
                    {
                        objects_to_deselect.emplace_back(selected_object);
                    }
                }
            }

            commands.push(new SelectObjectsCommand(
                {}, std::move(objects_to_deselect)), true);
        }
        else
        {
            commands.push(new SelectObjectsCommand(
                {object}, selected_objects), true);
        }
    }
}

void ObjectSelector::confirm_keyboard_move()
{
    state = State::INITIAL;
    front_plane_switch->node = nullptr;

    commands.push(new MoveObjectsCommand(RouteObject::get_selected_objects(),
        total_translation), false);
}

void ObjectSelector::cancel_keyboard_move()
{
    state = State::INITIAL;

    for (const auto& object : RouteObject::get_selected_objects())
    {
        object->move(-total_translation);
    }

    front_plane_switch->node = nullptr;
}

void ObjectSelector::confirm_keyboard_rotate()
{
    state = State::INITIAL;
    front_plane_switch->node = nullptr;

    // commands.push(new MoveObjectsCommand(RouteObject::get_selected_objects(),
    //     total_translation), false);
}

void ObjectSelector::cancel_keyboard_rotate()
{
    state = State::INITIAL;

    // for (const auto& object : RouteObject::get_selected_objects())
    // {
    //     object->move(-total_translation);
    // }

    front_plane_switch->node = nullptr;
}

void ObjectSelector::confirm_keyboard_scale()
{
    state = State::INITIAL;
    front_plane_switch->node = nullptr;

    // commands.push(new MoveObjectsCommand(RouteObject::get_selected_objects(),
    //     total_translation), false);
}

void ObjectSelector::cancel_keyboard_scale()
{
    state = State::INITIAL;

    // for (const auto& object : RouteObject::get_selected_objects())
    // {
    //     object->move(-total_translation);
    // }

    front_plane_switch->node = nullptr;
}
