#ifndef EDITOR_GUI_OBJECTS_REF_WINDOW_H
#define EDITOR_GUI_OBJECTS_REF_WINDOW_H

#include <vsgImGui/imgui.h>

struct EditorContext;
struct gui_settings_t;

class ObjectsRefWindow
{
public:
    ObjectsRefWindow(EditorContext& editor_context,
                     ImGuiWindowFlags window_flags = 0);
    void show() const;

private:
    gui_settings_t& gui_settings;
    const ImGuiWindowFlags window_flags;
};

#endif // EDITOR_GUI_OBJECTS_REF_WINDOW_H
