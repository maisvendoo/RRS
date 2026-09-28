#ifndef EDITOR_STATE_H2
#define EDITOR_STATE_H2

#include <string>

struct EditorContext;

class State
{
public:
    explicit State(EditorContext& editor_context);
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

    virtual void draw_gui() const;
    virtual void fill_status_bar() const;

    const std::string& get_name() const;

protected:
    EditorContext& editor_context;
    std::string name;
};

#endif // EDITOR_STATE_H2
