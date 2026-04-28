#include "graphics/shader_funcs.h"

#include "Journal.h"
#include "filesystem.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/utils/ShaderSet.h>

#include <QString>

#include <string>

vsg::ref_ptr<vsg::ShaderStage> read_shader(
    const char* shaders_dir,
    const char* filename,
    vsg::ref_ptr<const vsg::Options> options
)
{
    const FileSystem& fs{FileSystem::getInstance()};
    const std::string shader_path{fs.combinePath(shaders_dir, filename)};

    const auto shader{vsg::read_cast<vsg::ShaderStage>(shader_path, options)};
    if (!shader)
    {
        Journal::instance()->warning(QString{"Failed to load shader %1"}
            .arg(shader_path));
    }

    return shader;
}

void configure_shader_set(
    const char* shaders_dir,
    const char* vert_shader_filename,
    const char* frag_shader_filename,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
)
{
    configure_shader_set(
        read_shader(shaders_dir, vert_shader_filename, options),
        read_shader(shaders_dir, frag_shader_filename, options),
        shader_set_name,
        shader_set
    );
}

void configure_shader_set(
    const char* shaders_dir,
    const char* vert_shader_filename,
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
)
{
    configure_shader_set(
        read_shader(shaders_dir, vert_shader_filename, options),
        frag_shader,
        shader_set_name,
        shader_set
    );
}

void configure_shader_set(
    const char* shaders_dir,
    vsg::ref_ptr<vsg::ShaderStage> vert_shader,
    const char* frag_shader_filename,
    vsg::ref_ptr<const vsg::Options> options,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
)
{
    configure_shader_set(
        vert_shader,
        read_shader(shaders_dir, frag_shader_filename, options),
        shader_set_name,
        shader_set
    );
}

void configure_shader_set(
    vsg::ref_ptr<vsg::ShaderStage> vert_shader,
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
)
{
    if (!vert_shader || !frag_shader)
    {
        Journal::instance()->warning(QString{"Using default %1 shader set"}
            .arg(shader_set_name));

        return;
    }

    shader_set->stages.front() = vert_shader;
    shader_set->stages.back() = frag_shader;

    shader_set->variants.clear();

    Journal::instance()->info(QString{"Shader set %1 configured"}
        .arg(shader_set_name));
}
