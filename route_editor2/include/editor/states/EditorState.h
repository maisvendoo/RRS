#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <vsg/ui/KeyEvent.h>

class EditorState
{
public:
    virtual ~EditorState();

    virtual void fill_status_bar() const;

    virtual void handle_key_press(vsg::KeySymbol key) const;

    virtual void handle_key_release(vsg::KeySymbol key) const;

    virtual void handle_mouse_move(int delta_x, int delta_y) const;

    virtual void update(double delta_time) const;
};

#endif // EDITOR_STATE_H
