#include "editor/states/KeyboardTranslateState.h"

#include "editor/Camera.h"
#include "editor/commands/CommandManager.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/StaticObjectManager.h"
#include "editor/commands/TranslateObjectsCommand.h"
#include "editor/states/TransformState.h"

#include <vsg/maths/vec3.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <memory>
#include <utility>

KeyboardTranslateState::KeyboardTranslateState(EditorContext& editor_ctx)
    : TransformState(editor_ctx)
{
    name = "KeyboardTranslateState";
}

KeyboardTranslateState::~KeyboardTranslateState() = default;

void KeyboardTranslateState::on_activate()
{
    TransformState::on_activate();
    prev_intersection = begin_intersection;
}

void KeyboardTranslateState::handle_mouse_move()
{
    const auto& static_object_manager = editor_ctx.object_manager;
    auto& selected_objects = static_object_manager->get_selected_objects();

    const vsg::dvec3 world_intersection = calculate_world_intersection();
    const vsg::dvec3 translation = world_intersection - prev_intersection;
    prev_intersection = world_intersection;

    selected_objects.for_each([&](std::size_t id) -> void {
        static_object_manager->move_object(id, translation);
    });
}

void KeyboardTranslateState::confirm_transform() const
{
    const auto& selected_objects = editor_ctx.selected_objects;
    const auto& command_manager = editor_ctx.command_manager;
    const auto& state_manager = editor_ctx.state_manager;

    auto command = std::make_unique<TranslateObjectsCommand>(editor_ctx,
        selected_objects, prev_intersection - begin_intersection);
    command_manager->push(std::move(command));
    state_manager->defer_switch_to(STATE_BASIC);
}
