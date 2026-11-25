#ifndef EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H
#define EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H

#include <vsgImGui/imgui.h>

class Camera;
struct EditorContext;
struct camera_settings_t;

class CameraSettingsWindow
{
public:
    explicit CameraSettingsWindow(EditorContext &editor_ctx,
                                  ImGuiWindowFlags window_flags = 0);
    void show() const;

private:
    const Camera &camera;
    camera_settings_t &camera_settings;
    bool &show_camera_settings;
    const ImGuiWindowFlags window_flags;
};

#endif // EDITOR_GUI_CAMERA_SETTINGS_WINDOW_H
