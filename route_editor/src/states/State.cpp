#include "states/State.h"

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

State::State(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : mouse(mouse)
    , keyboard(keyboard)
    , state_manager(state_manager)
{
}

State::~State() = default;

void State::on_activate()
{
}

void State::on_deactivate()
{
}

void State::handle_key_press()
{
}

void State::handle_key_release()
{
}

void State::handle_button_press()
{
}

void State::handle_button_release()
{
}

void State::handle_mouse_move()
{
}

void State::handle_mouse_scroll()
{
}

void State::update(double delta_time)
{
    static_cast<void>(delta_time);
}

void State::draw_gui() const
{
}

void State::fill_status_bar() const
{
    ImGui::Text("%s", name.c_str());
    ImGui::SameLine();
}

const std::string& State::get_name() const
{
    return name;
}
