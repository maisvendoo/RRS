#include "editor/states/State.h"

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

State::State(EditorContext& editor_context)
    : editor_context(editor_context)
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

void State::update([[maybe_unused]] double delta_time)
{
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
