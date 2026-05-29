#ifndef EDITOR_GUI_SETTINGS_H
#define EDITOR_GUI_SETTINGS_H

class CfgReader;

struct gui_settings_t
{
    float font_size;
    bool is_editable;
    bool show_objects_ref;
    bool show_route_map;
    bool show_stations_conf;
    bool show_waypoints_conf;
    bool show_key_bindings;
    bool show_camera_settings;
    bool show_topology;
    bool show_selected_objects_properties;
    bool show_commands;

    /**
     * @brief Construct a new gui_settings_t object
     *        and initialize it with default values.
     */
    gui_settings_t();

    /**
     * @brief Read the GUI settings.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the GUI settings in Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_GUI_SETTINGS_H
