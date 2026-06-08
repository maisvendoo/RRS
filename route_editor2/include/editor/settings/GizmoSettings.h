#ifndef EDITOR_GIZMO_SETTINGS_H
#define EDITOR_GIZMO_SETTINGS_H

#include <vsg/maths/vec3.h>

class CfgReader;

struct gizmo_settings_t
{
    vsg::vec3 axis_x_color;
    vsg::vec3 axis_y_color;
    vsg::vec3 axis_z_color;
    float arrow_length;
    float arrow_thickness;
    float opacity;

    /**
     * @brief Place the gizmo at the center of the object's bounds
     * instead of placing it at the objects's origin.
     */
    bool to_center;

    /**
     * @brief Construct a new gizmo_settings_t object
     *        and initialize it with default values.
     */
    gizmo_settings_t();

    /**
     * @brief Read the gizmo settings from a config file.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing the editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the gizmo settings in the Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_GIZMO_SETTINGS_H
