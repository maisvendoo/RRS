#ifndef SELECTION_HANDLER_H
#define SELECTION_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class SelectionState;

class SelectionHandler : public vsg::Inherit<vsg::Visitor, SelectionHandler>
{
public:
    SelectionHandler();

private:
    SelectionState* gizmo_grab_selection_state = nullptr;
    SelectionState* gizmo_rotate_selection_state = nullptr;
    SelectionState* gizmo_scale_selection_state = nullptr;
    SelectionState* keyboard_grab_selection_state = nullptr;
    SelectionState* keyboard_rotate_selection_state = nullptr;
    SelectionState* keyboard_scale_selection_state = nullptr;
    SelectionState* prepare_grab_selection_state = nullptr;
    SelectionState* prepare_rotate_selection_state = nullptr;
    SelectionState* prepare_scale_selection_state = nullptr;

    SelectionState* prev_selection_state = nullptr;
    SelectionState* curr_selection_state = nullptr;
};

#endif // SELECTION_HANDLER_H
