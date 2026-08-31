#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "EditorState.h"

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

#include <cstddef>
#include <string>

class Camera;
class CommandManager;
struct EditorContext;
class Gizmo;
struct KeyBindings;
class Route;
class RouteObject;
class StateManager;
struct camera_settings_t;
struct gui_settings_t;

namespace vsg
{

class CommandBuffer;

}

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(
        EditorContext& context,
        camera_settings_t& camera_settings,
        gui_settings_t& gui_settings,
        const KeyBindings& key_bindings,
        StateManager& state_manager,
        const vsg::ref_ptr<Camera>& camera,
        EditorState& editor_state,
        CommandManager& command_manager,
        const vsg::ref_ptr<Route>& route,
        std::string& route_dir,
        const vsg::ref_ptr<Gizmo>& gizmo
    );

    ~EditorGui();

    void record(vsg::CommandBuffer& command_buffer) const override;

private:
    void show_objects_ref() const;
    void show_route_map() const;
    void show_stations_conf() const;
    void show_waypoints_conf() const;

    void show_key_bindings() const;
    void show_camera_settings() const;
    void show_topology() const;

    void show_selected_objects_properties() const;
    void show_commands() const;

    void add_object(
        const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
        const std::string& label
    ) const;

    void save_objects_matrixes() const;

    void handle_translation_drag(
        std::size_t index,
        const vsg::ref_ptr<RouteObject>& object,
        bool& dragging
    ) const;

    void handle_rotation_drag(
        std::size_t index,
        const vsg::ref_ptr<RouteObject>& object,
        bool& dragging
    ) const;

    void handle_scale_drag(
        std::size_t index,
        const vsg::ref_ptr<RouteObject>& object,
        bool& dragging
    ) const;

private:
    EditorContext& context_;
    camera_settings_t& camera_settings;
    gui_settings_t& gui_settings;
    const KeyBindings& key_bindings;
    StateManager& state_manager;
    const vsg::ref_ptr<Camera>& camera;
    EditorState& editor_state;
    CommandManager& command_manager;
    const vsg::ref_ptr<Route>& route;
    std::string& route_dir;
    const vsg::ref_ptr<Gizmo>& gizmo;

    ImGuiWindowFlags window_flags_;
    ImGuiViewport* viewport;

private:
    void add_ttf_font(
        const char* filename,
        float size_pixels,
        const ImFontConfig* font_cfg = nullptr,
        const ImWchar* glyph_ranges = nullptr
    );

    void draw_status_bar() const;

    void draw_load_route_file_dialog() const;

    void draw_invalid_route_popup() const;
};

#endif // EDITOR_GUI_H
