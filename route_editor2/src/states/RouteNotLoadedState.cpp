#include "editor/states/RouteNotLoadedState.h"

#include <vsgImGui/imgui.h>

RouteNotLoadedState::~RouteNotLoadedState() = default;

void RouteNotLoadedState::fill_status_bar() const
{
    ImGui::Text("Route is not loaded");
}
