#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "EditorState.h"
#include "KeyBindings.h"
#include "Route.h"
#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/KeyEvent.h>
#include <vsgImGui/imgui.h>

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
        EditorState& editor_state,
        const KeyBindings& key_bindings,
        vsg::ref_ptr<Route> route,
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
    EditorState& editor_state;
    const KeyBindings& key_bindings;
    bool show_demo_window = false;
    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::MatrixTransform>* selected_object = nullptr;
    vsg::ref_ptr<vsg::Perspective> perspective = nullptr;

    settings_t& settings;
    ImGuiWindowFlags window_flags = 0;
};

#endif // EDITOR_GUI_H
