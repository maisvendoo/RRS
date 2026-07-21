#include "ObjectSelector.h"

#include "Action.h"
#include "Camera.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Keyboard.h"
#include "Mask.h"
#include "Mouse.h"
#include "Route.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "SingleSwitch.h"
#include "commands/CommandList.h"
#include "commands/DeleteObjects.h"
#include "commands/PasteObjects.h"
#include "commands/RotateObjects.h"
#include "commands/ScaleObjects.h"
#include "commands/SelectObjects.h"
#include "commands/TranslateObjects.h"

#include <vsg/core/Mask.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <cmath>

ObjectSelector::ObjectSelector(EditorContext& context, const vsg::ref_ptr<Mouse>& mouse)
    : context_(context)
    , mouse(mouse)
{
    context.gizmo = Gizmo::create(context, context.settings.gizmo_settings, context.intersection_handler);

    front_plane_switch_ = SingleSwitch::create(
        vsg::Mask{MASK_CLICKABLE}, nullptr);

    context.scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE},
        context.gizmo);

    context.scene_graph->addChild(vsg::Mask{MASK_CLICKABLE},
        front_plane_switch_);
}

void ObjectSelector::apply(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;

    if (mouse->is_rmb_pressed())
    {
        return;
    }

    if (state_ != State::INITIAL)
    {
        return;
    }

    const RouteObjects& selected_objects = context_.selected_objects;
    if (selected_objects.empty())
    {
        return;
    }

    const auto keyboard = context_.keyboard;

    if (keyboard->pressed(ACTION_COPY_OBJECTS))
    {
        context_.copied_objects = selected_objects;
        return;
    }
    else if (keyboard->pressed(ACTION_PASTE_OBJECTS))
    {
        context_.commands.push(new PasteObjects(context_), true);
        return;
    }
    else if (keyboard->pressed(ACTION_DELETE_OBJECTS))
    {
        context_.commands.push(new DeleteObjects(context_), true);
        return;
    }

    const bool pressed_action_move = keyboard->pressed(ACTION_TRANSLATE_OBJECTS);
    const bool pressed_action_rotate = keyboard->pressed(ACTION_ROTATE_OBJECTS);
    const bool pressed_action_scale = keyboard->pressed(ACTION_SCALE_OBJECTS);

    if (!pressed_action_move && !pressed_action_rotate &&
        !pressed_action_scale)
    {
        return;
    }

    const auto front_plane = context_.camera->create_front_plane(
        context_.gizmo->get_curr_pos(), &front_plane_up_);

    const auto intersector = context_.intersection_handler->apply_(
        mouse->get_pos());

    if (!intersector)
    {
        return;
    }

    front_plane->accept(*intersector);

    const auto intersection =
        context_.intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    prev_intersect_pos_ = intersection->worldIntersection;

    intersector->intersections.clear();

    total_translation_ = {0.0, 0.0, 0.0};
    total_rotation_rad_ = 0.0;
    total_scale_ = {1.0, 1.0, 1.0};

    context_.compile_infos.emplace_back(CompileInfo{front_plane_switch_, front_plane});

    for (const auto& object : selected_objects)
    {
        object->save_matrix();
    }

    if (pressed_action_move)
    {
        state_ = State::KEYBOARD_GRAB;
    }
    else if (pressed_action_rotate)
    {
        state_ = State::KEYBOARD_ROTATE;
    }
    else if (pressed_action_scale)
    {
        state_ = State::KEYBOARD_SCALE;
    }
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    if (state_ != State::INITIAL)
    {
        if (mouse->is_lmb_pressed())
        {
            confirm_keyboard_transformation();
            return;
        }
        else if (mouse->is_rmb_pressed())
        {
            cancel_keyboard_transformation();
            return;
        }
    }

    const RouteObjects& selected_objects = context_.selected_objects;

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects.empty() && context_.gizmo->handle_intersections())
    {
        return;
    }

    const auto intersector =
        context_.intersection_handler->get_lmb_intersector();

    if (!intersector)
    {
        return;
    }

    context_.scene_graph->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        // If we clicked on empty space without shift
        // while there were selected objects,
        // deselect them all
        if (!selected_objects.empty() &&
            !context_.keyboard->get_shift_state())
        {
            SelectObjects* const select_objects_command =
                new SelectObjects(context_);

            select_objects_command->objects_to_deselect = selected_objects;
            select_objects_command->update_description();

            context_.commands.push(select_objects_command, true);
        }

        return;
    }

    const auto intersection =
        context_.intersection_handler->get_closest_intersection(intersector);

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
    context_.gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    context_.gizmo->apply(moveEvent);

    if (state_ == State::INITIAL || !front_plane_switch_->node)
    {
        return;
    }

    const auto intersector = context_.intersection_handler->apply_(moveEvent);

    front_plane_switch_->node->accept(*intersector);

    const auto intersection =
        context_.intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    intersector->intersections.clear();

    switch (state_)
    {
        case State::KEYBOARD_GRAB:
        {
            const vsg::dvec3 translation = world_intersection - prev_intersect_pos_;

            prev_intersect_pos_ = world_intersection;
            total_translation_ += translation;

            for (const auto& object : context_.selected_objects)
            {
                object->move(translation);
            }

            return;
        }
        case State::KEYBOARD_ROTATE:
        {
            const vsg::dvec3& gizmo_pos = context_.gizmo->get_curr_pos();

            if (world_intersection == gizmo_pos)
            {
                return;
            }

            const vsg::dvec3 prev_vec = vsg::normalize(prev_intersect_pos_ - gizmo_pos);
            const vsg::dvec3 curr_vec = vsg::normalize(world_intersection - gizmo_pos);

            prev_intersect_pos_ = world_intersection;

            double prev_acos = acos(vsg::dot(prev_vec, front_plane_up_));
            double curr_acos = acos(vsg::dot(curr_vec, front_plane_up_));

            const vsg::dvec3& front = context_.camera->get_front();

            if (prev_vec != front_plane_up_ && prev_vec != -front_plane_up_ &&
                vsg::dot(vsg::cross(prev_vec, front_plane_up_), front) < 0.0)
            {
                prev_acos = 2 * vsg::PI - prev_acos;
            }

            if (curr_vec != front_plane_up_ && curr_vec != -front_plane_up_ &&
                vsg::dot(vsg::cross(curr_vec, front_plane_up_), front) < 0.0)
            {
                curr_acos = 2 * vsg::PI - curr_acos;
            }

            for (const auto& object : context_.selected_objects)
            {
                const double rotation_rad = prev_acos - curr_acos;
                total_rotation_rad_ += rotation_rad;

                object->rotate_around_pivot(gizmo_pos, front, rotation_rad, object->matrix);
            }

            return;
        }
        case State::KEYBOARD_SCALE:
        {
            const vsg::dvec3& gizmo_pos = context_.gizmo->get_curr_pos();

            if (world_intersection == gizmo_pos)
            {
                return;
            }

            const vsg::dvec3 prev_vec = prev_intersect_pos_ - gizmo_pos;
            const vsg::dvec3 curr_vec = world_intersection - gizmo_pos;

            prev_intersect_pos_ = world_intersection;

            const double scale_value = vsg::length(curr_vec) / vsg::length(prev_vec);

            const vsg::dvec3 scale = {scale_value, scale_value, scale_value};
            total_scale_ *= scale;

            for (const auto& object : context_.selected_objects)
            {
                object->scale_relative_to_pivot(gizmo_pos, scale, object->matrix);
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
        new SelectObjects(context_);

    RouteObjects& objects_to_select =
        select_objects_command->objects_to_select;

    RouteObjects& objects_to_deselect =
        select_objects_command->objects_to_deselect;

    if (context_.keyboard->get_shift_state())
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
        const RouteObjects& selected_objects = context_.selected_objects;

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

    context_.commands.push(select_objects_command, true);
}

void ObjectSelector::confirm_keyboard_transformation()
{
    switch (state_)
    {
        case State::KEYBOARD_GRAB:
        {
            context_.commands.push(new TranslateObjects(
                context_, context_.selected_objects, total_translation_), false);

            break;
        }
        case State::KEYBOARD_ROTATE:
        {
            context_.commands.push(
                new RotateObjects(
                    context_, context_.selected_objects,
                    context_.gizmo->get_curr_pos(),
                    context_.camera->get_front(),
                    total_rotation_rad_
                ),
                false
            );

            break;
        }
        case State::KEYBOARD_SCALE:
        {
            context_.commands.push(new ScaleObjects(context_, context_.selected_objects,
                context_.gizmo->get_curr_pos(), total_scale_), false);

            break;
        }
        default:
        {
            break;
        }
    }

    state_ = State::INITIAL;
    front_plane_switch_->node = nullptr;
}

void ObjectSelector::cancel_keyboard_transformation()
{
    for (const auto& object : context_.selected_objects)
    {
        object->set_matrix(object->get_initial_matrix());
    }

    state_ = State::INITIAL;
    front_plane_switch_->node = nullptr;
}
