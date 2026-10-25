#include "editor/gui/CameraSettingsWindow.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"

#include <vsgImGui/imgui.h>

#include <cfloat>

CameraSettingsWindow::CameraSettingsWindow(EditorContext& editor_ctx,
    ImGuiWindowFlags window_flags)
    : camera(editor_ctx.camera)
    , camera_settings(editor_ctx.camera_settings)
    , gui_settings(editor_ctx.gui_settings)
    , window_flags(window_flags)
{
}

void CameraSettingsWindow::show() const
{
    if (!gui_settings.show_camera_settings)
        return;

    constexpr double min_drag_value = 0.0;

    ImGui::Begin("Camera settings", &gui_settings.show_camera_settings,
        window_flags);

    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::DragScalar("##CameraMoveSpeed", ImGuiDataType_Double,
        &camera_settings.move_speed, 1.0f, &min_drag_value, nullptr,
        "Move speed: %.3f");

    ImGui::DragScalar("##CameraRotateSpeed", ImGuiDataType_Double,
        &camera_settings.rotate_speed, 1.0f, &min_drag_value, nullptr,
        "Rotate speed: %.3f");

    ImGui::DragScalar("##CameraZoomPower", ImGuiDataType_Double,
        &camera_settings.zoom_power, 1.0f, &min_drag_value, nullptr,
        "Zoom power: %.3f");

    if (ImGui::SliderScalar("##CameraFovY", ImGuiDataType_Double,
            &camera_settings.fovy, &camera_settings.fovy_min,
            &camera_settings.fovy_max, "FovY: %.3f"))
        camera->get_perspective()->fieldOfViewY = camera_settings.fovy;

    ImGui::PopItemWidth();

    ImGui::End();
}
