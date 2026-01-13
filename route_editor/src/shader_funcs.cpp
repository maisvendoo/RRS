#include "shader_funcs.h"

#include "filesystem.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/utils/ShaderSet.h>

#include <vulkan/vulkan_core.h>

#include <cstdio>

vsg::ref_ptr<vsg::ShaderStage> read_shader(
    VkShaderStageFlagBits stage,
    const char* shaders_dir,
    const char* filename,
    vsg::ref_ptr<const vsg::Options> options
)
{
    FileSystem& fs = FileSystem::getInstance();
    const auto shader_path = fs.combinePath(shaders_dir, filename);

    const auto shader = vsg::ShaderStage::read(
        stage, "main", shader_path, options);

    if (!shader)
    {
        std::printf("Failed to load shader %s\n", shader_path.c_str());
        return nullptr;
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
        read_shader(VK_SHADER_STAGE_VERTEX_BIT,
            shaders_dir, vert_shader_filename, options),
        read_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
            shaders_dir, frag_shader_filename, options),
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
        read_shader(VK_SHADER_STAGE_VERTEX_BIT,
            shaders_dir, vert_shader_filename, options),
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
        read_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
            shaders_dir, frag_shader_filename, options),
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
        std::printf("Using default %s shader set\n", shader_set_name);
        return;
    }

    shader_set->stages.front() = vert_shader;
    shader_set->stages.back() = frag_shader;

    // Очищаем все встроенные сохраненные варианты настроек
    shader_set->variants.clear();

    std::printf("Shader set %s configured\n", shader_set_name);
}
