#ifndef EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H
#define EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

class Camera;
struct EditorContext;
struct camera_settings_t;
struct gui_settings_t;

class CameraSettingsWindow
{
public:
    CameraSettingsWindow(EditorContext& editor_context,
                         ImGuiWindowFlags window_flags = 0);
    void show() const;

private:
    const vsg::ref_ptr<Camera>& camera;
    camera_settings_t& camera_settings;
    gui_settings_t& gui_settings;
    const ImGuiWindowFlags window_flags;
};

#endif // EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H
