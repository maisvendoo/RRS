#include "editor/states/KeyboardTranslateState.h"

#include "editor/Camera.h"
#include "editor/commands/CommandManager.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/commands/TranslateObjectsCommand.h"
#include "editor/states/KeyboardTransformState.h"

#include <vsg/maths/vec3.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <memory>
#include <utility>

KeyboardTranslateState::KeyboardTranslateState(EditorContext& editor_context)
    : KeyboardTransformState(editor_context)
{
    name = "KeyboardTranslateState";
}

KeyboardTranslateState::~KeyboardTranslateState() = default;

void KeyboardTranslateState::on_activate()
{
    KeyboardTransformState::on_activate();
    prev_intersection = begin_intersection;
}

void KeyboardTranslateState::handle_mouse_move()
{
    const auto& selected_objects = editor_context.selected_objects;

    const vsg::dvec3 world_intersection = calculate_world_intersection();
    const vsg::dvec3 translation = world_intersection - prev_intersection;
    prev_intersection = world_intersection;

    for (const auto& object : selected_objects)
    {
        object->move(translation);
    }
}

void KeyboardTranslateState::confirm_transform() const
{
    const auto& selected_objects = editor_context.selected_objects;
    const auto& command_manager = editor_context.command_manager;
    const auto& state_manager = editor_context.state_manager;

    auto command = std::make_unique<TranslateObjectsCommand>(editor_context,
        selected_objects, prev_intersection - begin_intersection);
    command_manager->push(std::move(command));
    state_manager->defer_switch_to(STATE_BASIC);
}
