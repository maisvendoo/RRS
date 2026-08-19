#include "editor/settings/GuiSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <core/string_funcs.h>

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
    cfg.getBool(section, "ShowSelectedObjectsProperties",
        show_selected_objects_properties);
    cfg.getBool(section, "ShowCommands", show_commands);
}

void gui_settings_t::print_in_journal() const
{
    Journal* const journal = Journal::instance();

    journal->debug("GUI settings:");
    journal->debug("    font_size: " + QString::number(font_size));
    journal->debug("    is_editable: " + to_qstring(is_editable));
    journal->debug("    show_objects_ref: " + to_qstring(show_objects_ref));
    journal->debug("    show_route_map: " + to_qstring(show_route_map));
    journal->debug("    show_stations_conf: " + to_qstring(show_stations_conf));
    journal->debug("    show_waypoints_conf: " +
        to_qstring(show_waypoints_conf));
    journal->debug("    show_key_bindings: " + to_qstring(show_key_bindings));
    journal->debug("    show_camera_settings: " +
        to_qstring(show_camera_settings));
    journal->debug("    show_topology: " + to_qstring(show_topology));
    journal->debug("    show_selected_objects_properties: " +
        to_qstring(show_selected_objects_properties));
    journal->debug("    show_commands: " + to_qstring(show_commands));
}
