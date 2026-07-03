#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

class EditorState
{
public:
    virtual ~EditorState();

    virtual void fill_status_bar() const;

    virtual void handle_key_press() const;
    virtual void handle_key_release() const;
    virtual void handle_button_press() const;
    virtual void handle_button_release() const;
    virtual void handle_mouse_move();

    virtual void update(double delta_time) const;
};

#endif // EDITOR_STATE_H
