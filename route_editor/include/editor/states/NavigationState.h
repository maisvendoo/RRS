#ifndef NAVIGATION_STATE_H
#define NAVIGATION_STATE_H

#include "editor/states/State.h"

class NavigationState : public State
{
public:
    NavigationState(EditorContext& editor_context);
    virtual ~NavigationState() override;

    virtual void on_activate() override;

    virtual void handle_key_press() override;
    virtual void handle_key_release() override;

    virtual void handle_button_release() override;
    virtual void handle_mouse_move() override;

    virtual void update(double delta_time) override;
};

#endif // NAVIGATION_STATE_H
