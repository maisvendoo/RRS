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

ObjectSelector::ObjectSelector(EditorContext& context)
    : context(context)
{
    gizmo = Gizmo::create(context);

    front_plane_switch = SingleSwitch::create(
        vsg::Mask{MASK_CLICKABLE}, nullptr);

    context.scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE}, gizmo);
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
    if (!selected_objects.empty() && gizmo->handle_intersections())
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
            context.commands.push(
                new SelectObjectsCommand(context, {}, selected_objects), true);
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
            const auto& selected_objects = context.selected_objects;

            vsg::vec3 center = {0.0f, 0.0f, 0.0f};
            for (const auto& object : selected_objects)
            {
                center += object->get_translation();
            }
            center /= static_cast<float>(selected_objects.size());

            if (world_intersection == center)
            {
                return;
            }

            // const auto print_vec3 = [&](const char* name, vsg::vec3 v) -> void
            // {
            //     std::printf("%s: %10.3f %10.3f %10.3f\n", name, v.x, v.y, v.z);
            // };

            const auto print_int = [&](const char* name, int i) -> void
            {
                std::printf("%s: %d\n", name, i);
            };

            const auto print_float = [&](const char* name, float f) -> void
            {
                std::printf("%s: %10.3f\n", name, f);
            };

            const auto vec_a = vsg::normalize(prev_intersect_pos - center);
            const auto vec_b = vsg::normalize(world_intersection - center);

            std::printf("-------------------------\n");
            // print_vec3("vec_a", vec_a);
            // print_vec3("vec_b", vec_b);

            prev_intersect_pos = world_intersection;

            const auto dot_a = vsg::dot(vec_a, front_plane_up);
            const auto dot_b = vsg::dot(vec_b, front_plane_up);

            // print_float("dot_a", dot_a);
            // print_float("dot_b", dot_b);

            const auto acos_a = std::acos(dot_a);
            const auto acos_b = std::acos(dot_b);

            // print_float("acos_a", acos_a);
            // print_float("acos_b", acos_b);

            int dir_a;
            int dir_b;

            if (vec_a == front_plane_up || vec_a == -front_plane_up)
            {
                dir_a = 1;
            }
            else
            {
                const auto cross_a = vsg::cross(vec_a, front_plane_up);
                if (vsg::dot(cross_a, front) >= 0.0f)
                {
                    dir_a = 1;
                }
                else
                {
                    dir_a = -1;
                }
            }

            if (vec_b == front_plane_up || vec_b == -front_plane_up)
            {
                dir_b = 1;
            }
            else
            {
                const auto cross_b = vsg::cross(vec_b, front_plane_up);
                if (vsg::dot(cross_b, front) >= 0.0f)
                {
                    dir_b = 1;
                }
                else
                {
                    dir_b = -1;
                }
            }

            print_int("dir_a", dir_a);
            print_int("dir_b", dir_b);

            auto deg_a = vsg::degrees(acos_a);
            if (dir_a == -1)
            {
                deg_a = 360.0f - deg_a;
            }

            auto deg_b = vsg::degrees(acos_b);
            if (dir_b == -1)
            {
                deg_b = 360.0f - deg_b;
            }

            print_float("deg_a", deg_a);
            print_float("deg_b", deg_b);

            auto diff = deg_a - deg_b;
            if (diff > 180.0f)
            {
                diff -= 360.0f;
            }
            else if (diff < -180.0f)
            {
                diff += 360.0f;
            }

            print_float("diff", diff);

            for (const auto& object : selected_objects)
            {
                object->rotate_around_pivot(center, front * diff, object->matrix);
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

    if (state != State::KEYBOARD_GRAB && !selected_objects.empty() &&
        context.keyboard_handler->get_binding_state(ACTION_MOVE_OBJECTS))
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

        state = State::KEYBOARD_GRAB;
        front_plane_switch->node = front_plane;

        vsg::updateViewer(*viewer, compile_result);
    }
}

vsg::ref_ptr<Gizmo> ObjectSelector::get_gizmo() const
{
    return gizmo;
}

void ObjectSelector::select_object(RouteObject* object)
{
    if (context.keyboard_handler->get_any_shift_state())
    {
        if (object->get_is_selected())
        {
            context.commands.push(new SelectObjectsCommand(context, {}, {object}), true);
        }
        else
        {
            context.commands.push(new SelectObjectsCommand(context, {object}, {}), true);
        }
    }
    else
    {
        const auto& selected_objects = context.selected_objects;

        if (selected_objects.empty())
        {
            context.commands.push(new SelectObjectsCommand(context, {object}, {}), true);
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

            context.commands.push(new SelectObjectsCommand(context,
                {}, std::move(objects_to_deselect)), true);
        }
        else
        {
            context.commands.push(new SelectObjectsCommand(context,
                {object}, selected_objects), true);
        }
    }
}

void ObjectSelector::confirm_keyboard_move()
{
    state = State::INITIAL;
    front_plane_switch->node = nullptr;

    context.commands.push(new MoveObjectsCommand(context, context.selected_objects,
        total_translation), false);
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
