#include "editor/settings/GizmoSettings.h"

#include <CfgReader.h>

#include <QString>

gizmo_settings_t::gizmo_settings_t()
    : axis_x_color(1.0f, 0.0f, 0.0f)
    , axis_y_color(0.0f, 1.0f, 0.0f)
    , axis_z_color(0.0f, 0.0f, 1.0f)
    , arrow_length(5.0f)
    , arrow_thickness(0.1f)
    , opacity(1.0f)
    , to_center(false)
{
}

void gizmo_settings_t::read(CfgReader& cfg)
{
    const QString section = "Gizmo";

    cfg.getFloat(section, "XAxisColorR", axis_x_color.r);
    cfg.getFloat(section, "XAxisColorG", axis_x_color.g);
    cfg.getFloat(section, "XAxisColorB", axis_x_color.b);
    cfg.getFloat(section, "YAxisColorR", axis_y_color.r);
    cfg.getFloat(section, "YAxisColorG", axis_y_color.g);
    cfg.getFloat(section, "YAxisColorB", axis_y_color.b);
    cfg.getFloat(section, "ZAxisColorR", axis_z_color.r);
    cfg.getFloat(section, "ZAxisColorG", axis_z_color.g);
    cfg.getFloat(section, "ZAxisColorB", axis_z_color.b);
    cfg.getFloat(section, "ArrowLength", arrow_length);
    cfg.getFloat(section, "ArrowThickness", arrow_thickness);
    cfg.getFloat(section, "Opacity", opacity);
    cfg.getBool(section, "ToCenter", to_center);
}
