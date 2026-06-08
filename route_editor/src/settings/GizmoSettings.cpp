#include "settings/GizmoSettings.h"

#include <CfgReader.h>

#include <QString>

gizmo_settings_t::gizmo_settings_t()
    : arrow_x_color(1.0f, 0.0f, 0.0f)
    , arrow_y_color(0.0f, 1.0f, 0.0f)
    , arrow_z_color(0.0f, 0.0f, 1.0f)
    , arrow_length(5.0f)
    , arrow_thickness(0.1f)
    , opacity(1.0f)
    , to_center(false)
{
}

void gizmo_settings_t::read(CfgReader& cfg)
{
    const QString section = "Gizmo";

    cfg.getFloat(section, "XAxisColorR", arrow_x_color.r);
    cfg.getFloat(section, "XAxisColorG", arrow_x_color.g);
    cfg.getFloat(section, "XAxisColorB", arrow_x_color.b);
    cfg.getFloat(section, "YAxisColorR", arrow_y_color.r);
    cfg.getFloat(section, "YAxisColorG", arrow_y_color.g);
    cfg.getFloat(section, "YAxisColorB", arrow_y_color.b);
    cfg.getFloat(section, "ZAxisColorR", arrow_z_color.r);
    cfg.getFloat(section, "ZAxisColorG", arrow_z_color.g);
    cfg.getFloat(section, "ZAxisColorB", arrow_z_color.b);
    cfg.getFloat(section, "ArrowLength", arrow_length);
    cfg.getFloat(section, "ArrowThickness", arrow_thickness);
    cfg.getFloat(section, "Opacity", opacity);
    cfg.getBool(section, "ToCenter", to_center);
}
