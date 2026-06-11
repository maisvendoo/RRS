#include "editor/states/RouteNotSelectedState.h"

#include <vsgImGui/imgui.h>

RouteNotSelectedState::~RouteNotSelectedState() = default;

void RouteNotSelectedState::fill_status_bar() const
{
    ImGui::Text("Route is not selected");
}
