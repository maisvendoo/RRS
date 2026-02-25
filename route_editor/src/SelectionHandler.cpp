#include "SelectionHandler.h"

#include "GizmoGrabSelectionState.h"
#include "GizmoRotateSelectionState.h"
#include "GizmoScaleSelectionState.h"
#include "KeyboardGrabSelectionState.h"
#include "KeyboardRotateSelectionState.h"
#include "KeyboardScaleSelectionState.h"
#include "PrepareGrabSelectionState.h"
#include "PrepareRotateSelectionState.h"
#include "PrepareScaleSelectionState.h"

#include <vsg/ui/PointerEvent.h>

SelectionHandler::SelectionHandler()
    : gizmo_grab_selection_state(new GizmoGrabSelectionState)
    , gizmo_rotate_selection_state(new GizmoRotateSelectionState)
    , gizmo_scale_selection_state(new GizmoScaleSelectionState)
    , keyboard_grab_selection_state(new KeyboardGrabSelectionState)
    , keyboard_rotate_selection_state(new KeyboardRotateSelectionState)
    , keyboard_scale_selection_state(new KeyboardScaleSelectionState)
    , prepare_grab_selection_state(new PrepareGrabSelectionState)
    , prepare_rotate_selection_state(new PrepareRotateSelectionState)
    , prepare_scale_selection_state(new PrepareScaleSelectionState)
    , selection_state(prepare_grab_selection_state)
{
}

SelectionHandler::~SelectionHandler()
{
    delete gizmo_grab_selection_state;
    delete gizmo_rotate_selection_state;
    delete gizmo_scale_selection_state;
    delete keyboard_grab_selection_state;
    delete keyboard_rotate_selection_state;
    delete keyboard_scale_selection_state;
    delete prepare_grab_selection_state;
    delete prepare_rotate_selection_state;
    delete prepare_scale_selection_state;
}

void SelectionHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    selection_state->apply(buttonPress);
}
