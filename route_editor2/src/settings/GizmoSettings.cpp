#include "editor/settings/GizmoSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <core/string_funcs.h>

#include <vsg/maths/vec3.h>

#include <QString>

/**
 * @brief Convert vec3 to QString.
 *
 * @param vec The vec3 object.
 * @return Formatted string representation of vec3 object.
 */
QString to_qstring(vsg::vec3 vec)
{
    return QString("{%1, %2, %3}").arg(vec.x).arg(vec.y).arg(vec.z);
}

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

void gizmo_settings_t::print_in_journal() const
{
    Journal* const journal = Journal::instance();

    journal->debug("Gizmo settings:");
    journal->debug("    axis_x_color: " + to_qstring(axis_x_color));
    journal->debug("    axis_y_color: " + to_qstring(axis_y_color));
    journal->debug("    axis_z_color: " + to_qstring(axis_z_color));
    journal->debug("    arrow_length: " + QString::number(arrow_length));
    journal->debug("    arrow_thickness: " + QString::number(arrow_thickness));
    journal->debug("    opacity: " + QString::number(opacity));
    journal->debug("    to_center: " + to_qstring(to_center));
}
