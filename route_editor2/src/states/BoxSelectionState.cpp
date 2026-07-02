#include "editor/states/BoxSelectionState.h"

#include <vsgImGui/imgui.h>

BoxSelectionState::~BoxSelectionState() = default;

void BoxSelectionState::fill_status_bar() const
{
    ImGui::Text("Box selection state");
}
