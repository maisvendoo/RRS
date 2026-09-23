#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsgImGui/imgui.h>

#include <cstddef>
#include <map>

class RouteObject;
struct EditorContext;

namespace vsg
{

class CommandBuffer;

}

/**
 * @brief Состояние перетаскивания отдельного объекта в окне свойств.
 *
 * Раньше total_translation/total_rotation_deg/total_scale/dragging были
 * static-переменными внутри функций и были общими для всех объектов,
 * из-за чего дельты смешивались между объектами.
 */
struct ObjectDragState
{
    bool dragging = false;
    vsg::dvec3 total_translation = {0.0, 0.0, 0.0};
    vsg::dvec3 total_rotation_deg = {0.0, 0.0, 0.0};
    vsg::dvec3 total_scale = {1.0, 1.0, 1.0};
};

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(EditorContext& context);
    ~EditorGui();

    void record(vsg::CommandBuffer& command_buffer) const override;

private:
    void select_route() const;
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
        vsg::ref_ptr<vsg::PagedLOD> paged_lod,
        const std::string& label
    ) const;

    void save_objects_matrixes() const;

    /// Получить состояние перетаскивания конкретного объекта
    /// (создаётся при первом обращении)
    ObjectDragState& get_drag_state(const RouteObject* object) const;

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

private:
    EditorContext& context_;

    ImGuiWindowFlags window_flags_;

    /// Состояния перетаскивания по каждому отображаемому объекту
    mutable std::map<const RouteObject*, ObjectDragState> drag_states_;
};

#endif // EDITOR_GUI_H
