#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>

struct gui_settings_t;

class ImGuiViewport;

namespace vsg
{

class CommandBuffer;

}

class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    explicit EditorGui(const gui_settings_t& gui_settings);

    virtual void record(vsg::CommandBuffer& commandBuffer) const override;

private:
    ImGuiViewport* viewport;
};

#endif // EDITOR_GUI_H
