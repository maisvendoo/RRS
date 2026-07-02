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
}

void BoxSelectionState::handle_button_release() const
{
    if (!(mouse->get_button_mask() & vsg::BUTTON_MASK_1))
    {
        state_manager.defer_switch_to_basic_editor_state();
    }
}
