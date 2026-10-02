#include "editor/states/KeyboardScaleState.h"

#include "editor/Camera.h"
#include "editor/commands/CommandManager.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/commands/RotateObjectsCommand.h"
#include "editor/commands/ScaleObjectsCommand.h"
#include "editor/states/TransformState.h"

#include <vsg/maths/vec3.h>

KeyboardScaleState::KeyboardScaleState(EditorContext& editor_context)
    : TransformState(editor_context)
{
    name = "KeyboardScaleState";
}

KeyboardScaleState::~KeyboardScaleState() = default;

void KeyboardScaleState::on_activate()
{
    const auto& gizmo = editor_context.gizmo;

    TransformState::on_activate();
    scale = {1.0, 1.0, 1.0};
    gizmo_pos = gizmo->get_curr_pos();
}

void KeyboardScaleState::handle_mouse_move()
{
    const auto& selected_objects = editor_context.selected_objects;

    const vsg::dvec3 world_intersection = calculate_world_intersection();
    if (vsg::length(world_intersection - gizmo_pos) < 1.0e-6f)
    {
        return;
    }

    const vsg::dvec3 begin_vec = begin_intersection - gizmo_pos;
    const vsg::dvec3 curr_vec = world_intersection - gizmo_pos;

    const double scale_value = vsg::length(curr_vec) / vsg::length(begin_vec);
    scale = {scale_value, scale_value, scale_value};

    for (const auto& object : selected_objects)
    {
        object->set_matrix(object->get_initial_matrix());
        object->scale_relative_to_pivot(gizmo_pos, scale, object->matrix);
    }
}

void KeyboardScaleState::confirm_transform() const
{
    const auto& selected_objects = editor_context.selected_objects;
    const auto& gizmo = editor_context.gizmo;
    const auto& command_manager = editor_context.command_manager;
    const auto& state_manager = editor_context.state_manager;

    auto command = std::make_unique<ScaleObjectsCommand>(editor_context,
        selected_objects, gizmo->get_curr_pos(), scale);
    command_manager->push(std::move(command));
    state_manager->defer_switch_to(STATE_BASIC);
}
