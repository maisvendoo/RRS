#ifndef GRAPHICS_SHADER_FUNCS_H
#define GRAPHICS_SHADER_FUNCS_H

#include <vsg/core/ref_ptr.h>

namespace vsg
{

class Options;
class ShaderSet;
class ShaderStage;

}

vsg::ref_ptr<vsg::ShaderStage> read_shader(
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

#endif // GRAPHICS_SHADER_FUNCS_H
