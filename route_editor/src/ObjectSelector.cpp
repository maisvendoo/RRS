#include "ObjectSelector.h"

#include "Action.h"
#include "CameraHandler.h"
#include "commands/CommandList.h"
#include "commands/DeleteObjects.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "MouseHandler.h"
#include "commands/TranslateObjects.h"
#include "commands/PasteObjects.h"
#include "commands/RotateObjects.h"
#include "Route.h"
#include "RouteObject.h"
#include "commands/ScaleObjects.h"
#include "SceneGraph.h"
#include "commands/SelectObjects.h"
#include "SingleSwitch.h"

#include <vsg/core/Mask.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <cmath>

ObjectSelector::ObjectSelector(EditorContext& context)
    : context(context)
{
    context.gizmo = Gizmo::create(context);

    front_plane_switch = SingleSwitch::create(
        vsg::Mask{MASK_CLICKABLE}, nullptr);

    context.scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE},
        context.gizmo);

    context.scene_graph->addChild(vsg::Mask{MASK_CLICKABLE},
        front_plane_switch);
}

void ObjectSelector::apply(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;

    if (context.mouse_handler->get_is_rmb_pressed())
    {
        return;
    }

    if (state != State::INITIAL)
    {
        return;
    }

    const RouteObjects& selected_objects = context.selected_objects;
    if (selected_objects.empty())
    {
        return;
    }

    const auto keyboard_handler = context.keyboard_handler;

    const auto get_binding_state = [keyboard_handler](Action action) -> bool
    {
        return keyboard_handler->get_binding_state(action);
    };

    if (get_binding_state(ACTION_COPY_OBJECTS))
    {
        context.copied_objects = selected_objects;
        return;
    }
    else if (get_binding_state(ACTION_PASTE_OBJECTS))
    {
        context.commands.push(new PasteObjects(context), true);
        return;
    }
    else if (get_binding_state(ACTION_DELETE_OBJECTS))
    {
        context.commands.push(new DeleteObjects(context), true);
        return;
    }

    const bool pressed_action_move = get_binding_state(ACTION_MOVE_OBJECTS);
    const bool pressed_action_rotate = get_binding_state(ACTION_ROTATE_OBJECTS);
    const bool pressed_action_scale = get_binding_state(ACTION_SCALE_OBJECTS);

    if (!pressed_action_move && !pressed_action_rotate &&
        !pressed_action_scale)
    {
        return;
    }

    const auto front_plane = context.camera_handler->create_front_plane(
        context.gizmo->get_curr_pos(), &front_plane_up);

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

    prev_intersect_pos = static_cast<vsg::vec3>(
        intersection->worldIntersection);

    intersector->intersections.clear();

    total_translation = {0.0f, 0.0f, 0.0f};
    total_rotation_rad = 0.0f;
    total_scale = {1.0f, 1.0f, 1.0f};

    context.compile_infos.emplace_back(CompileInfo{
        front_plane_switch, front_plane});

    for (const auto& object : selected_objects)
    {
        object->save_matrix();
    }

    if (pressed_action_move)
    {
        state = State::KEYBOARD_GRAB;
    }
    else if (pressed_action_rotate)
    {
        state = State::KEYBOARD_ROTATE;
    }
    else if (pressed_action_scale)
    {
        state = State::KEYBOARD_SCALE;
    }
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    if (state != State::INITIAL)
    {
        if (context.mouse_handler->get_is_lmb_pressed())
        {
            confirm_keyboard_transformation();
            return;
        }
        else if (context.mouse_handler->get_is_rmb_pressed())
        {
            cancel_keyboard_transformation();
            return;
        }
    }

    const RouteObjects& selected_objects = context.selected_objects;

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects.empty() && context.gizmo->handle_intersections())
    {
        return;
    }

    const auto intersector =
        context.intersection_handler->get_lmb_intersector();

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
            SelectObjects* const select_objects_command =
                new SelectObjects(context);

            select_objects_command->objects_to_deselect = selected_objects;
            select_objects_command->update_description();

            context.commands.push(select_objects_command, true);
        }

        return;
    }

    const auto intersection =
        context.intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if (const RouteObject* const object = node->cast<RouteObject>())
        {
            select_object(vsg::ref_ptr(const_cast<RouteObject*>(object)));
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

    if (state == State::INITIAL || !front_plane_switch->node)
    {
        return;
    }

    const auto intersector = context.intersection_handler->apply_(moveEvent);

    front_plane_switch->node->accept(*intersector);

    const auto intersection =
        context.intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    const auto world_intersection = static_cast<vsg::vec3>(
        intersection->worldIntersection);

    intersector->intersections.clear();

    switch (state)
    {
        case State::KEYBOARD_GRAB:
        {
            const vsg::vec3 translation = world_intersection -
                prev_intersect_pos;

            prev_intersect_pos = world_intersection;
            total_translation += translation;

            for (const auto& object : context.selected_objects)
            {
                object->move(static_cast<vsg::dvec3>(translation));
            }

            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            const vsg::vec3 gizmo_pos = context.gizmo->get_curr_pos();

            if (world_intersection == gizmo_pos)
            {
                return;
            }

            const vsg::vec3 prev_vec = vsg::normalize(
                prev_intersect_pos - gizmo_pos);

            const vsg::vec3 curr_vec = vsg::normalize(
                world_intersection - gizmo_pos);

            prev_intersect_pos = world_intersection;

            float prev_acos = std::acos(vsg::dot(prev_vec, front_plane_up));
            float curr_acos = std::acos(vsg::dot(curr_vec, front_plane_up));

            const vsg::vec3 front = static_cast<vsg::vec3>(
                context.camera_handler->get_front());

            if (prev_vec != front_plane_up && prev_vec != -front_plane_up &&
                vsg::dot(vsg::cross(prev_vec, front_plane_up), front) < 0.0f)
            {
                prev_acos = 2 * vsg::PI - prev_acos;
            }

            if (curr_vec != front_plane_up && curr_vec != -front_plane_up &&
                vsg::dot(vsg::cross(curr_vec, front_plane_up), front) < 0.0f)
            {
                curr_acos = 2 * vsg::PI - curr_acos;
            }

            for (const auto& object : context.selected_objects)
            {
                const float rotation_rad = prev_acos - curr_acos;
                total_rotation_rad += rotation_rad;

                object->rotate_around_pivot(gizmo_pos, front, rotation_rad,
                    object->matrix);
            }

            return;
        }
        case State::KEYBOARD_SCALE:
        {
            const vsg::vec3 gizmo_pos = context.gizmo->get_curr_pos();

            if (world_intersection == gizmo_pos)
            {
                return;
            }

            const vsg::vec3 prev_vec = prev_intersect_pos - gizmo_pos;
            const vsg::vec3 curr_vec = world_intersection - gizmo_pos;

            prev_intersect_pos = world_intersection;

            const float scale_value = vsg::length(curr_vec) /
                vsg::length(prev_vec);

            const vsg::vec3 scale = {scale_value, scale_value, scale_value};
            total_scale *= scale;

            for (const auto& object : context.selected_objects)
            {
                object->scale_relative_to_pivot(gizmo_pos, scale,
                    object->matrix);
            }

            return;
        }
        default:
        {
            return;
        }
    }
}

void ObjectSelector::select_object(vsg::ref_ptr<RouteObject> object)
{
    SelectObjects* const select_objects_command =
        new SelectObjects(context);

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

void ObjectSelector::confirm_keyboard_transformation()
{
    switch (state)
    {
        case State::KEYBOARD_GRAB:
        {
            context.commands.push(new TranslateObjects(
                context, total_translation), false);

            break;
        }
        case State::KEYBOARD_ROTATE:
        {
            context.commands.push(
                new RotateObjects(
                    context, context.gizmo->get_curr_pos(),
                    static_cast<vsg::vec3>(context.camera_handler->get_front()),
                    total_rotation_rad
                ),
                false
            );

            break;
        }
        case State::KEYBOARD_SCALE:
        {
            context.commands.push(new ScaleObjects(context,
                context.gizmo->get_curr_pos(), total_scale), false);

            break;
        }
        default:
        {
            break;
        }
    }

    state = State::INITIAL;
    front_plane_switch->node = nullptr;
}

void ObjectSelector::cancel_keyboard_transformation()
{
    for (const auto& object : context.selected_objects)
    {
        object->set_matrix(object->get_initial_matrix());
    }

    state = State::INITIAL;
    front_plane_switch->node = nullptr;
}
