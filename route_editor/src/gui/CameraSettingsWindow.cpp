#include "editor/gui/CameraSettingsWindow.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsgImGui/imgui.h>

#include <cfloat>

#define FLOAT_FORMAT "%.3f"

static bool drag_scalar(const char *label, void *data, const char *format)
{
    static constexpr double min = 0.0;
    return ImGui::DragScalar(label, ImGuiDataType_Double, data, 1.0f, &min,
                             nullptr, format);
}

CameraSettingsWindow::CameraSettingsWindow(EditorContext &editor_ctx,
                                           ImGuiWindowFlags window_flags)
    : camera(*editor_ctx.camera)
    , camera_settings(editor_ctx.camera_settings)
    , show_camera_settings(editor_ctx.gui_settings.show_camera_settings)
    , window_flags(window_flags)
{
}

void CameraSettingsWindow::show() const
{
    if (!show_camera_settings)
    {
        return;
    }

    ImGui::Begin("Camera settings", &show_camera_settings, window_flags);
    ImGui::PushItemWidth(-FLT_MIN);

    drag_scalar("##CameraMoveSpeed", &camera_settings.move_speed,
                "Move speed: " FLOAT_FORMAT);

    drag_scalar("##CameraRotateSpeed", &camera_settings.rotate_speed,
                "Rotate speed: " FLOAT_FORMAT);

    drag_scalar("##CameraZoomPower", &camera_settings.zoom_power,
                "Zoom power: " FLOAT_FORMAT);

    if (ImGui::SliderScalar("##CameraFovY", ImGuiDataType_Double,
                            &camera_settings.fovy, &camera_settings.fovy_min,
                            &camera_settings.fovy_max, "FovY: " FLOAT_FORMAT))
    {
        camera.get_perspective()->fieldOfViewY = camera_settings.fovy;
    }

    ImGui::PopItemWidth();
    ImGui::End();
}
