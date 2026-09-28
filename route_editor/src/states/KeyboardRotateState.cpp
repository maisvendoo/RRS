#include "editor/states/KeyboardRotateState.h"

#include "editor/Camera.h"
#include "editor/commands/CommandManager.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/commands/RotateObjectsCommand.h"
#include "editor/states/KeyboardTransformState.h"

#include <vsg/maths/vec3.h>

KeyboardRotateState::KeyboardRotateState(EditorContext& editor_context)
    : KeyboardTransformState(editor_context)
{
    name = "KeyboardRotateState";
}

KeyboardRotateState::~KeyboardRotateState() = default;

void KeyboardRotateState::on_activate()
{
    const auto& gizmo = editor_context.gizmo;
    const auto& camera = editor_context.camera;

    KeyboardTransformState::on_activate();
    rotation_rad = 0.0;
    gizmo_pos = gizmo->get_curr_pos();
    camera_front = camera->get_front();
}

void KeyboardRotateState::handle_mouse_move()
{
    const auto& camera = editor_context.camera;
    const vsg::dvec3& camera_up = camera->get_up();
    const auto& selected_objects = editor_context.selected_objects;

    const vsg::dvec3 world_intersection = calculate_world_intersection();
    if (vsg::length(world_intersection - gizmo_pos) < 1.0e-6f)
    {
        return;
    }

    const vsg::dvec3 begin_vec = vsg::normalize(begin_intersection - gizmo_pos);
    const vsg::dvec3 curr_vec = vsg::normalize(world_intersection - gizmo_pos);

    double begin_acos = std::acos(vsg::dot(begin_vec, camera_up));
    double curr_acos = std::acos(vsg::dot(curr_vec, camera_up));

    if (begin_vec != camera_up && begin_vec != -camera_up &&
        vsg::dot(vsg::cross(begin_vec, camera_up), camera_front) < 0.0)
    {
        begin_acos = 2 * vsg::PI - begin_acos;
    }

    if (curr_vec != camera_up && curr_vec != -camera_up &&
        vsg::dot(vsg::cross(curr_vec, camera_up), camera_front) < 0.0)
    {
        curr_acos = 2 * vsg::PI - curr_acos;
    }

    for (const auto& object : selected_objects)
    {
        rotation_rad = begin_acos - curr_acos;
        object->set_matrix(object->get_initial_matrix());
        object->rotate_around_pivot(gizmo_pos, camera_front, rotation_rad,
            object->matrix);
    }
}

void KeyboardRotateState::confirm_transform() const
{
    const auto& selected_objects = editor_context.selected_objects;
    const auto& gizmo = editor_context.gizmo;
    const auto& camera = editor_context.camera;
    const auto& command_manager = editor_context.command_manager;
    const auto& state_manager = editor_context.state_manager;

    auto command = std::make_unique<RotateObjectsCommand>(
        editor_context, selected_objects, gizmo->get_curr_pos(),
        camera->get_front(), rotation_rad);
    command_manager->push(std::move(command));
    state_manager->defer_switch_to(STATE_BASIC);
}
