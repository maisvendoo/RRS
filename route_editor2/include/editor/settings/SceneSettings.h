#ifndef EDITOR_SCENE_SETTINGS_H
#define EDITOR_SCENE_SETTINGS_H

class CfgReader;

struct scene_settings_t
{
    /// Maximum number of light sources in a scene.
    int num_lights;

    /**
     * @brief Construct a new scene_settings_t object
     *        and initialize it with default values.
     */
    scene_settings_t();

    /**
     * @brief Read the scene settings from a config file.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing the editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the scene settings in the Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_SCENE_SETTINGS_H
