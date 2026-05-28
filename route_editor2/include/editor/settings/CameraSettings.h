#ifndef EDITOR_CAMERA_SETTINGS_H
#define EDITOR_CAMERA_SETTINGS_H

class CfgReader;

struct camera_settings_t
{
    /// Near clip plane distance.
    double zNear;
    /// Far clip plane distance.
    double view_distance;
    /// Vertical field of view.
    double fovy;
    /// Minimum vertical field of view.
    double fovy_min;
    /// Maximum vertical field of view.
    double fovy_max;

    double initial_height;
    double move_speed;
    double rotate_speed;
    double zoom_power;

    /**
     * @brief Construct a new camera_settings_t object and
     *        initialize it with default values.
     */
    camera_settings_t();

    /**
     * @brief Read the camera settings.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the camera settings in Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_CAMERA_SETTINGS_H
