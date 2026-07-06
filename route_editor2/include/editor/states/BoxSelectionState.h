#ifndef EDITOR_BOX_SELECTION_STATE_H
#define EDITOR_BOX_SELECTION_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

class Mouse;
class StateManager;

namespace vsg
{

class Options;
class StateGroup;
class Switch;

};

class BoxSelectionState : public EditorState
{
public:
    int begin_x;
    int begin_y;
    int end_x;
    int end_y;

public:
    BoxSelectionState(
        const vsg::ref_ptr<Mouse>& mouse,
        StateManager& state_manager,
        const vsg::ref_ptr<vsg::Options>& vsg_options
    );

    virtual ~BoxSelectionState() override;

    virtual void fill_status_bar() const override;

    virtual void handle_button_release() const override;

    virtual void handle_mouse_move() override;

    const vsg::ref_ptr<vsg::Switch>& get_switch_node() const;

private:
    const vsg::ref_ptr<Mouse>& mouse;
    StateManager& state_manager;
    const vsg::ref_ptr<vsg::Options>& vsg_options;

    vsg::ref_ptr<vsg::Switch> switch_node;
    vsg::ref_ptr<vsg::StateGroup> state_group;
};

#endif // EDITOR_BOX_SELECTION_STATE_H
