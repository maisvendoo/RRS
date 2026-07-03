#include "editor/states/BoxSelectionState.h"

#include "editor/Mouse.h"
#include "editor/StateManager.h"

#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

BoxSelectionState::BoxSelectionState(
    const vsg::ref_ptr<Mouse>& mouse,
    StateManager& state_manager
)
    : mouse(mouse)
    , state_manager(state_manager)
{
}

BoxSelectionState::~BoxSelectionState() = default;

void BoxSelectionState::fill_status_bar() const
{
    ImGui::Text("Box selection state");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("Begin pos: %dx%d\n", begin_x, begin_y);
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("End pos: %dx%d\n", end_x, end_y);
}

void BoxSelectionState::handle_button_release() const
{
    if (!(mouse->get_button_mask() & vsg::BUTTON_MASK_1))
    {
        state_manager.defer_switch_to_basic_editor_state();
    }
}

void BoxSelectionState::handle_mouse_move()
{
    end_x = mouse->get_pos_x();
    end_y = mouse->get_pos_y();
}
