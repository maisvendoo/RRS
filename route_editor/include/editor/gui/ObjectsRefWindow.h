#ifndef EDITOR_GUI_OBJECTS_REF_WINDOW_H
#define EDITOR_GUI_OBJECTS_REF_WINDOW_H

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

class Camera;
class CommandManager;
struct EditorContext;
struct gui_settings_t;

class ObjectsRef;

class ObjectsRefWindow
{
public:
    ObjectsRefWindow(EditorContext& editor_ctx,
                     ImGuiWindowFlags window_flags = 0);

    void show() const;

private:
    const vsg::ref_ptr<Camera> camera;
    CommandManager* const command_manager;
    gui_settings_t& gui_settings;
    ObjectsRef& objects_ref;
    const ImGuiWindowFlags window_flags;
};

#endif // EDITOR_GUI_OBJECTS_REF_WINDOW_H
