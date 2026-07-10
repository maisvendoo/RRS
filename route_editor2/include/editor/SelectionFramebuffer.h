#ifndef EDITOR_SELECTION_FRAMEBUFFER_H
#define EDITOR_SELECTION_FRAMEBUFFER_H

#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

namespace vsg
{

class Context;

}

class SelectionFramebuffer
{
public:
    SelectionFramebuffer(
        const vsg::ref_ptr<vsg::Context>& context,
        VkExtent2D window_extent
    );
};

#endif // EDITOR_SELECTION_FRAMEBUFFER_H
