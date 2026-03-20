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

ObjectSelector::ObjectSelector(EditorContext& context)
    : context(context)
{
    context.gizmo = Gizmo::create(context);

    front_plane_switch = SingleSwitch::create(
        vsg::Mask{MASK_CLICKABLE}, nullptr);

    context.scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE}, context.gizmo);
    context.scene_graph->addChild(vsg::Mask{MASK_CLICKABLE}, front_plane_switch);
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
            if (context.mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_move();
            }
            else if (context.mouse_handler->get_is_rmb_pressed())
            {
                cancel_keyboard_move();
            }

            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            if (context.mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_rotate();
            }
            else if (context.mouse_handler->get_is_rmb_pressed())
            {
                cancel_keyboard_rotate();
            }

            return;
        }
        case State::KEYBOARD_SCALE:
        {
            if (context.mouse_handler->get_is_lmb_pressed())
            {
                confirm_keyboard_scale();
            }
            else if (context.mouse_handler->get_is_rmb_pressed())
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

    const auto& selected_objects = context.selected_objects;

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects.empty() && context.gizmo->handle_intersections())
    {
        return;
    }

    const auto intersector = context.intersection_handler->get_lmb_intersector();
    if (!intersector)
    {
        return;
    }

    context.scene_graph->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        // If we clicked on empty space without shift
        // while there were selected objects,
        // deselect them all
        if (!selected_objects.empty() &&
            !context.keyboard_handler->get_any_shift_state())
        {
            SelectObjectsCommand* const select_objects_command =
                new SelectObjectsCommand(context);

            select_objects_command->objects_to_deselect = selected_objects;
            select_objects_command->update_description();

            context.commands.push(select_objects_command, true);
        }

        return;
    }

    const auto intersection = context.intersection_handler->get_closest_intersection(
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
    context.gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    context.gizmo->apply(moveEvent);

    const auto front_plane = front_plane_switch->node;
    if (!front_plane)
    {
        return;
    }

    const auto intersector = context.intersection_handler->apply_(moveEvent);

    front_plane->accept(*intersector);

    const auto intersection = context.intersection_handler->get_closest_intersection(intersector);

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
            const auto translation = world_intersection - prev_intersect_pos;
            prev_intersect_pos = world_intersection;
            total_translation += translation;

            for (const auto& object : context.selected_objects)
            {
                object->move(translation);
            }

            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            const RouteObjects& selected_objects = context.selected_objects;

            vsg::vec3 center = {0.0f, 0.0f, 0.0f};
            for (const RouteObject* const object : selected_objects)
            {
                center += object->get_translation();
            }
            center /= static_cast<float>(selected_objects.size());

            if (world_intersection == center)
            {
                return;
            }

            const vsg::vec3 vec_a = vsg::normalize(prev_intersect_pos - center);
            const vsg::vec3 vec_b = vsg::normalize(world_intersection - center);

            prev_intersect_pos = world_intersection;

            float acos_a = std::acos(vsg::dot(vec_a, front_plane_up));
            float acos_b = std::acos(vsg::dot(vec_b, front_plane_up));

            if (vec_a != front_plane_up && vec_a != -front_plane_up &&
                vsg::dot(vsg::cross(vec_a, front_plane_up), front) < 0.0f)
            {
                acos_a = 2 * vsg::PI - acos_a;
            }

            if (vec_b != front_plane_up && vec_b != -front_plane_up &&
                vsg::dot(vsg::cross(vec_b, front_plane_up), front) < 0.0f)
            {
                acos_b = 2 * vsg::PI - acos_b;
            }

            for (const auto& object : selected_objects)
            {
                object->rotate_around_pivot(center, front, acos_a - acos_b, object->matrix);
            }

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

    const auto& selected_objects = context.selected_objects;

    const bool pressed_action_move =
        context.keyboard_handler->get_binding_state(ACTION_MOVE_OBJECTS);

    const bool pressed_action_rotate =
        context.keyboard_handler->get_binding_state(ACTION_ROTATE_OBJECTS);

    if (state == State::INITIAL && !selected_objects.empty() &&
        (pressed_action_move || pressed_action_rotate))
    {
        vsg::vec3 begin_pos = {0.0f, 0.0f, 0.0f};
        for (const auto& object : selected_objects)
        {
            begin_pos += object->get_translation();
        }
        begin_pos /= static_cast<float>(selected_objects.size());

        const auto front_plane = context.camera_handler->create_front_plane(begin_pos,
            &front_plane_up);

        front = context.camera_handler->get_front();

        const auto intersector = context.intersection_handler->apply_(
            context.mouse_handler->get_pos());

        if (!intersector)
        {
            return;
        }

        front_plane->accept(*intersector);

        const auto intersection =
            context.intersection_handler->get_closest_intersection(intersector);

        if (!intersection)
        {
            return;
        }

        begin_intersect_pos = static_cast<vsg::vec3>(
            intersection->worldIntersection);

        prev_intersect_pos = begin_intersect_pos;
        total_translation = {0.0f, 0.0f, 0.0f};

        const auto viewer = context.viewer;
        const auto compile_manager = viewer->compileManager;
        const auto compile_result = compile_manager->compile(front_plane);

        front_plane_switch->node = front_plane;

        vsg::updateViewer(*viewer, compile_result);

        if (pressed_action_move)
        {
            state = State::KEYBOARD_GRAB;
        }
        else if (pressed_action_rotate)
        {
            state = State::KEYBOARD_ROTATE;
        }
    }
}

void ObjectSelector::select_object(RouteObject* object)
{
    SelectObjectsCommand* const select_objects_command =
        new SelectObjectsCommand(context);

    RouteObjects& objects_to_select =
        select_objects_command->objects_to_select;

    RouteObjects& objects_to_deselect =
        select_objects_command->objects_to_deselect;

    if (context.keyboard_handler->get_any_shift_state())
    {
        if (object->get_is_selected())
        {
            objects_to_deselect.emplace_back(object);
        }
        else
        {
            objects_to_select.emplace_back(object);
        }
    }
    else
    {
        const RouteObjects& selected_objects = context.selected_objects;

        if (selected_objects.empty())
        {
            objects_to_select.emplace_back(object);
        }
        else if (object->get_is_selected())
        {
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
        }
        else
        {
            objects_to_select.emplace_back(object);
            objects_to_deselect = selected_objects;
        }
    }

    select_objects_command->update_description();

    context.commands.push(select_objects_command, true);
}

void ObjectSelector::confirm_keyboard_move()
{
    state = State::INITIAL;
    front_plane_switch->node = nullptr;

    context.commands.push(new MoveObjectsCommand(context, total_translation), false);
}

void ObjectSelector::cancel_keyboard_move()
{
    state = State::INITIAL;

    for (const auto& object : context.selected_objects)
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
