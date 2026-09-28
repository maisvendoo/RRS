#include "editor/states/KeyboardTransformState.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/editor_math.h"
#include "editor/states/State.h"

#include <vsg/app/Window.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

KeyboardTransformState::KeyboardTransformState(EditorContext& editor_context)
    : State(editor_context)
{
}

KeyboardTransformState::~KeyboardTransformState() = default;

void KeyboardTransformState::on_activate()
{
    begin_intersection = calculate_world_intersection();
}

void KeyboardTransformState::handle_key_press()
{
    const auto& keyboard = editor_context.keyboard;

    if (keyboard->pressed_once(vsg::KEY_Return))
    {
        confirm_transform();
    }
    else if (keyboard->pressed_once(vsg::KEY_Escape))
    {
        cancel_transform();
    }
}

void KeyboardTransformState::handle_button_press()
{
    const auto& mouse = editor_context.mouse;

    switch (mouse->get_button_mask())
    {
        case vsg::BUTTON_MASK_1:
        {
            confirm_transform();
            return;
        }
        case vsg::BUTTON_MASK_3:
        {
            cancel_transform();
            return;
        }
        default:
        {
            return;
        }
    }
}

void KeyboardTransformState::handle_mouse_move()
{
}

void KeyboardTransformState::confirm_transform() const
{
}

void KeyboardTransformState::cancel_transform() const
{
    const auto& selected_objects = editor_context.selected_objects;
    const auto& state_manager = editor_context.state_manager;

    for (const auto& object : selected_objects)
    {
        object->set_matrix(object->get_initial_matrix());
    }

    state_manager->defer_switch_to(STATE_BASIC);
}

vsg::dvec3 KeyboardTransformState::calculate_world_intersection() const
{
    const auto& mouse = editor_context.mouse;
    const auto& window = editor_context.window;
    const auto& camera = editor_context.camera;
    const auto& gizmo = editor_context.gizmo;

    vsg::dvec3 world_intersection;
    calculate_intersection_mouse_and_plane(mouse->get_x(), mouse->get_y(),
        window->extent2D(), camera->get_inverse_view_matrix(),
        camera->get_inverse_projection_matrix(), gizmo->get_curr_pos(),
        camera->get_front(), world_intersection);

    return world_intersection;
}
