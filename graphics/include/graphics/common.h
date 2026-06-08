#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

namespace vsg
{

class Options;

}

vsg::ref_ptr<vsg::Options> create_default_vsg_options();

VkSampleCountFlags get_vk_sample_count_flag(int samples);

#endif // GRAPHICS_COMMON_H
