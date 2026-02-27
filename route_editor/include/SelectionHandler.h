// #ifndef SELECTION_HANDLER_H
// #define SELECTION_HANDLER_H

// #include <vsg/core/Inherit.h>
// #include <vsg/core/Visitor.h>

// class SelectionState;

// namespace vsg
// {

// class ButtonPressEvent;
// class ButtonReleaseEvent;
// class KeyPressEvent;
// class KeyReleaseEvent;
// class MoveEvent;

// }

// class SelectionHandler : public vsg::Inherit<vsg::Visitor, SelectionHandler>
// {
// public:
//     SelectionHandler();
//     ~SelectionHandler();

//     virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
//     virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
//     virtual void apply(vsg::MoveEvent& moveEvent) override;
//     virtual void apply(vsg::KeyPressEvent& keyPress) override;
//     virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

// private:
//     SelectionState* gizmo_grab_selection_state = nullptr;
//     SelectionState* gizmo_rotate_selection_state = nullptr;
//     SelectionState* gizmo_scale_selection_state = nullptr;
//     SelectionState* keyboard_grab_selection_state = nullptr;
//     SelectionState* keyboard_rotate_selection_state = nullptr;
//     SelectionState* keyboard_scale_selection_state = nullptr;
//     SelectionState* prepare_grab_selection_state = nullptr;
//     SelectionState* prepare_rotate_selection_state = nullptr;
//     SelectionState* prepare_scale_selection_state = nullptr;

//     SelectionState* selection_state = nullptr;
// };

// #endif // SELECTION_HANDLER_H
