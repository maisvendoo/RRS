#include "ObjectSelector.h"

#include "Action.h"
#include "Camera.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Keyboard.h"
#include "Mask.h"
#include "Mouse.h"
#include "Route.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "commands/CommandList.h"
#include "commands/DeleteObjects.h"
#include "commands/PasteObjects.h"
#include "commands/RotateObjects.h"
#include "commands/ScaleObjects.h"
#include "commands/SelectObjects.h"
#include "commands/TranslateObjects.h"
#include "editor_math.h"

#include <vsg/core/Mask.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <cmath>
#include <vsg/utils/LineSegmentIntersector.h>

ObjectSelector::ObjectSelector(
    EditorContext& context,
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list,
    const vsg::ref_ptr<SceneGraph>& scene_graph,
    const vsg::ref_ptr<Route>& route,
    const VkExtent2D& window_extent,
    const vsg::ref_ptr<Gizmo>& gizmo
)
    : context_(context)
    , mouse(mouse)
    , keyboard(keyboard)
    , camera(camera)
    , command_list(command_list)
    , scene_graph(scene_graph)
    , route(route)
    , window_extent(window_extent)
    , gizmo(gizmo)
{
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

    if (keyboard->pressed(ACTION_COPY_OBJECTS))
    {
        context_.copied_objects = selected_objects;
        return;
    }
    else if (keyboard->pressed(ACTION_PASTE_OBJECTS))
    {
        command_list.push(new PasteObjects(context_, route, gizmo), true);
        return;
    }
    else if (keyboard->pressed(ACTION_DELETE_OBJECTS))
    {
        command_list.push(new DeleteObjects(context_, route, gizmo), true);
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

    double norm_mouse_x, norm_mouse_y;
    normalize_mouse_coordinates(mouse->get_x(), mouse->get_y(), window_extent,
        norm_mouse_x, norm_mouse_y);

    const vsg::dmat4& inv_view_mat = camera->get_inverse_view_matrix();
    const vsg::dmat4& inv_proj_mat = camera->get_inverse_projection_matrix();

    vsg::dvec3 mouse_world1, mouse_world2;
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 0.0,
        inv_view_mat, inv_proj_mat, mouse_world1);
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 1.0,
        inv_view_mat, inv_proj_mat, mouse_world2);

    calculate_intersection_line_and_plane(mouse_world1,
        mouse_world2 - mouse_world1, gizmo->get_curr_pos(), camera->get_front(),
        prev_intersect_pos_);

    total_translation_ = {0.0, 0.0, 0.0};
    total_rotation_rad_ = 0.0;
    total_scale_ = {1.0, 1.0, 1.0};

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

    if (!(buttonPress.button == 1 && buttonPress.mask == vsg::BUTTON_MASK_1))
    {
        return;
    }

    const auto intersector = vsg::LineSegmentIntersector::create(*camera,
        buttonPress.x, buttonPress.y);
    if (!intersector)
    {
        return;
    }
    intersector->traversalMask = MASK_CLICKABLE;

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects.empty() && gizmo->handle_intersections())
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
        if (!selected_objects.empty() && !keyboard->get_shift_state())
        {
            SelectObjects* const select_objects_command =
                new SelectObjects(context_, gizmo);

            select_objects_command->objects_to_deselect = selected_objects;
            select_objects_command->update_description();

            command_list.push(select_objects_command, true);
        }

        return;
    }

    for (const vsg::Node* const node : intersections.front()->nodePath)
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
    gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    gizmo->apply(moveEvent);

    if (state_ == State::INITIAL)
    {
        return;
    }

    double norm_mouse_x, norm_mouse_y;
    normalize_mouse_coordinates(mouse->get_x(), mouse->get_y(), window_extent,
        norm_mouse_x, norm_mouse_y);

    const vsg::dmat4& inv_view_mat = camera->get_inverse_view_matrix();
    const vsg::dmat4& inv_proj_mat = camera->get_inverse_projection_matrix();

    vsg::dvec3 mouse_world1, mouse_world2;
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 0.0,
        inv_view_mat, inv_proj_mat, mouse_world1);
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 1.0,
        inv_view_mat, inv_proj_mat, mouse_world2);

    vsg::dvec3 world_intersection;
    calculate_intersection_line_and_plane(mouse_world1,
        mouse_world2 - mouse_world1, gizmo->get_curr_pos(), camera->get_front(),
        world_intersection);

    const vsg::dvec3& camera_up = camera->get_up();

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
            const vsg::dvec3& gizmo_pos = gizmo->get_curr_pos();

            if (world_intersection == gizmo_pos)
            {
                return;
            }

            const vsg::dvec3 prev_vec = vsg::normalize(prev_intersect_pos_ - gizmo_pos);
            const vsg::dvec3 curr_vec = vsg::normalize(world_intersection - gizmo_pos);

            prev_intersect_pos_ = world_intersection;

            double prev_acos = acos(vsg::dot(prev_vec, camera_up));
            double curr_acos = acos(vsg::dot(curr_vec, camera_up));

            const vsg::dvec3& front = camera->get_front();

            if (prev_vec != camera_up && prev_vec != -camera_up &&
                vsg::dot(vsg::cross(prev_vec, camera_up), front) < 0.0)
            {
                prev_acos = 2 * vsg::PI - prev_acos;
            }

            if (curr_vec != camera_up && curr_vec != -camera_up &&
                vsg::dot(vsg::cross(curr_vec, camera_up), front) < 0.0)
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
            const vsg::dvec3& gizmo_pos = gizmo->get_curr_pos();

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
        new SelectObjects(context_, gizmo);

    RouteObjects& objects_to_select =
        select_objects_command->objects_to_select;

    RouteObjects& objects_to_deselect =
        select_objects_command->objects_to_deselect;

    if (keyboard->get_shift_state())
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

    command_list.push(select_objects_command, true);
}

void ObjectSelector::confirm_keyboard_transformation()
{
    switch (state_)
    {
        case State::KEYBOARD_GRAB:
        {
            command_list.push(new TranslateObjects(
                context_, context_.selected_objects, total_translation_), false);

            break;
        }
        case State::KEYBOARD_ROTATE:
        {
            command_list.push(
                new RotateObjects(
                    context_, context_.selected_objects,
                    gizmo->get_curr_pos(),
                    camera->get_front(),
                    total_rotation_rad_
                ),
                false
            );

            break;
        }
        case State::KEYBOARD_SCALE:
        {
            command_list.push(new ScaleObjects(context_, context_.selected_objects,
                gizmo->get_curr_pos(), total_scale_), false);

            break;
        }
        default:
        {
            break;
        }
    }

    state_ = State::INITIAL;
}

void ObjectSelector::cancel_keyboard_transformation()
{
    for (const auto& object : context_.selected_objects)
    {
        object->set_matrix(object->get_initial_matrix());
    }

    state_ = State::INITIAL;
}
