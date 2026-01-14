#ifndef SHADER_FUNCS_H
#define SHADER_FUNCS_H

#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

namespace vsg
{

class Options;
class ShaderSet;
class ShaderStage;

}

vsg::ref_ptr<vsg::ShaderStage> read_shader(
    VkShaderStageFlagBits stage,
    const char* shaders_dir,
    const char* filename,
    vsg::ref_ptr<const vsg::Options> options
);

void configure_shader_set(
    const char* shaders_dir,
    const char* vert_shader_filename,
    const char* frag_shader_filename,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
);

void configure_shader_set(
    const char* shaders_dir,
    const char* vert_shader_filename,
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
);

void configure_shader_set(
    const char* shaders_dir,
    vsg::ref_ptr<vsg::ShaderStage> vert_shader,
    const char* frag_shader_filename,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
);

void configure_shader_set(
    vsg::ref_ptr<vsg::ShaderStage> vert_shader,
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
);

#endif // SHADER_FUNCS_H
