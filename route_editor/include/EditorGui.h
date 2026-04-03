#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "EditorContext.h"

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

namespace vsg
{

class CommandBuffer;

}

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(EditorContext& context);

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

private:
    EditorContext& context;

    ImGuiWindowFlags window_flags;
};

#endif // EDITOR_GUI_H
