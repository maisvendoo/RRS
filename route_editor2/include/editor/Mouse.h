#ifndef EDITOR_MOUSE_H
#define EDITOR_MOUSE_H

class Mouse
{
public:
    bool is_lmb_pressed() const;
    bool is_mmb_pressed() const;
    bool is_rmb_pressed() const;

private:
    bool lmb_state;
    bool mmb_state;
    bool rmb_state;
};

#endif // EDITOR_MOUSE_H
