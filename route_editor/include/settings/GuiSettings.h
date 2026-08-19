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

    gui_settings_t();
    void read(CfgReader& cfg);
};

#endif // EDITOR_GUI_SETTINGS_H
