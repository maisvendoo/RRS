#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>

#include <string>

struct gui_settings_t;

class ImGuiViewport;
class StateManager;

namespace vsg
{

class CommandBuffer;

}

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(
        const gui_settings_t& gui_settings,
        StateManager& state_manager,
        std::string& route_dir
    );

    virtual void record(vsg::CommandBuffer& commandBuffer) const override;

private:
    StateManager& state_manager;
    std::string& route_dir;

    ImGuiViewport* viewport;

private:
    void add_ttf_font(const char* filename, float size);

    void draw_menu_bar() const;

    void draw_status_bar() const;

    void draw_load_route_file_dialog() const;

    void draw_invalid_route_popup() const;
};

#endif // EDITOR_GUI_H
