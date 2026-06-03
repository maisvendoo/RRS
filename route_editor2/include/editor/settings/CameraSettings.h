#ifndef EDITOR_CAMERA_SETTINGS_H
#define EDITOR_CAMERA_SETTINGS_H

class CfgReader;

struct camera_settings_t
{
    /// Near clipping plane distance.
    double zNear;

    /// Far clipping plane distance.
    double view_distance;

    /// Vertical field of view in degrees.
    double fovy_degrees;

    /// Minimum vertical field of view in degrees.
    double min_fovy_degrees;

    /// Maximum vertical field of view in degrees.
    double max_fovy_degrees;

    double initial_height;
    double move_speed;
    double rotate_speed;
    double zoom_power;

    /**
     * @brief Construct a new camera_settings_t object
     *        and initialize it with default values.
     */
    camera_settings_t();

    /**
     * @brief Read the camera settings from a config file.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing the editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the camera settings in the Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_CAMERA_SETTINGS_H
