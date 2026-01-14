#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

struct EditorParams;
struct settings_t;

namespace vsg
{

class CommandBuffer;
class Options;

}

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(
        vsg::ref_ptr<EditorParams> editor_params,
        settings_t& settings,
        vsg::ref_ptr<vsg::Options> options = {}
    );

    void record(vsg::CommandBuffer& command_buffer) const override;

private:
    void select_route() const;
    void show_objects_ref() const;
    void show_route_map() const;

    void show_key_bindings() const;
    void show_camera_settings() const;
    void show_topology() const;

    void show_selected_object_properties() const;

private:
    vsg::ref_ptr<EditorParams> editor_params;
    settings_t& settings;
    ImGuiWindowFlags window_flags = 0;
};

#endif // EDITOR_GUI_H
