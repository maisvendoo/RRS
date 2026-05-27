#include "editor/settings/GuiSettings.h"

#include <CfgReader.h>

#include <QString>

gui_settings_t::gui_settings_t()
    : font_size(20.0f)
    , is_editable(true)
    , show_objects_ref(true)
    , show_route_map(false)
    , show_stations_conf(true)
    , show_waypoints_conf(false)
    , show_key_bindings(true)
    , show_camera_settings(true)
    , show_topology(true)
    , show_selected_objects_properties(true)
    , show_commands(true)
{
}

void gui_settings_t::read(CfgReader& cfg)
{
    const QString section = "GUI";

    cfg.getFloat(section, "FontSize", font_size);
    cfg.getBool(section, "IsEditable", is_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowStationsConf", show_stations_conf);
    cfg.getBool(section, "ShowWaypointsConf", show_waypoints_conf);
    cfg.getBool(section, "ShowKeyBindings", show_key_bindings);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);
    cfg.getBool(
        section,
        "ShowSelectedObjectsProperties",
        show_selected_objects_properties
    );
    cfg.getBool(section, "ShowCommands", show_commands);
}
