#ifndef EDITOR_STATE_H2
#define EDITOR_STATE_H2

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

class State
{
public:
    State(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );

    virtual ~State();

    virtual void on_activate();

    virtual void on_deactivate();

    virtual void handle_key_press();

    virtual void handle_key_release();

    virtual void handle_button_press();

    virtual void handle_button_release();

    virtual void handle_mouse_move();

    virtual void handle_mouse_scroll();

    virtual void update(double delta_time);

    virtual void fill_status_bar() const;

    virtual const char* get_name() const;

protected:
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;
};

#endif // EDITOR_STATE_H2
