#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsgImGui/imgui.h>

#include <cstddef>
#include <map>
#include <string>

struct EditorContext;
struct gui_settings_t;
class RouteObject;

/**
 * @brief Состояние перетаскивания отдельного объекта в инспекторе
 *        (по-объектно, чтобы дельты не смешивались между объектами).
 */
struct ObjectDragState
{
    bool dragging = false;
    vsg::dvec3 total_translation = {0.0, 0.0, 0.0};
    vsg::dvec3 total_rotation_deg = {0.0, 0.0, 0.0};
    vsg::dvec3 total_scale = {1.0, 1.0, 1.0};
};

/// ImGui-интерфейс редактора
class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(EditorContext& context, const gui_settings_t& gui_settings);

    virtual void record(vsg::CommandBuffer& commandBuffer) const override;

private:
    void draw_menu_bar() const;

    void draw_status_bar() const;

    void draw_open_route_file_dialog() const;
    void draw_new_route_dir_dialog() const;
    void draw_new_route_popup() const;
    void draw_invalid_route_popup() const;

    void draw_objects_ref_window() const;
    void draw_route_map_window() const;
    void draw_stations_window() const;
    void draw_selected_objects_window() const;
    void draw_commands_window() const;
    void draw_progress_window() const;

    void add_object(const std::string& label) const;

    ObjectDragState& get_drag_state(const RouteObject* object) const;

    void save_objects_matrixes() const;

    void handle_translation_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    void handle_rotation_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    void handle_scale_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    /// Создать структуру каталогов нового маршрута и открыть его
    bool create_route_structure(const std::string& parent_dir,
        const std::string& name) const;

private:
    EditorContext& context_;
    const gui_settings_t& gui_settings;

    ImGuiViewport* viewport = nullptr;

    /// Родительский каталог для нового маршрута (окно New route)
    mutable std::string new_route_parent_dir;

    /// Имя нового маршрута (окно New route)
    mutable char new_route_name[256] = {};

    /// Состояния перетаскивания по каждому объекту-инспектору
    mutable std::map<const RouteObject*, ObjectDragState> drag_states_;
};

#endif // EDITOR_GUI_H
