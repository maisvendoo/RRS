#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "EditorState.h"
#include "RouteMap.h"
#include "StringMap.h"
#include "topology.h"
#include <filesystem>
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
        const vsg::KeySymbol* key_bindings,
        std::filesystem::path* route_dir,
        settings_t& settings,
        const Topology& topology,
        const StringMap& objects_ref,
        const RouteMap& route_map,
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
    const vsg::KeySymbol* key_bindings;
    bool show_demo_window = false;
    std::filesystem::path* route_dir = nullptr;
    const StringMap& objects_ref;
    const RouteMap& route_map;
    vsg::ref_ptr<vsg::MatrixTransform>* selected_object = nullptr;
    vsg::ref_ptr<vsg::Perspective> perspective = nullptr;
    const Topology& topology;

    settings_t& settings;
    ImGuiWindowFlags window_flags = 0;
};

#endif // EDITOR_GUI_H
