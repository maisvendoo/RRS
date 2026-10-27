#ifndef GRAPHICS_SHADER_FUNCS_H
#define GRAPHICS_SHADER_FUNCS_H

#include <vsg/core/ref_ptr.h>

#include <string>

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

/// Обёртка тонмаппинга фрагментного шейдера (пресеты High/Ultra,
/// ТЗ "Графика"): переименовывает первый "void main()" исходника в
/// rrs_tonemap_original_main() и дописывает в конец tonemap_glsl
/// (графикой принимается gfx::aces_tonemap_shader_fragment() — он
/// определяет новый void main(), использующий outColor).
///
/// Контракт tonemap_glsl: самодостаточный GLSL, объявляющий новый
/// main() и вызывающий rrs_tonemap_original_main(); имя выхода
/// фрагмента — out_color_name (по умолчанию "outColor", как во всех
/// шейдерах проекта в data/shaders).
///
/// Возвращает false и НЕ меняет шейдер, если инжект невозможен:
/// нет шейдера/GLSL-исходника (предкомпилированный SPIR-V не
/// модифицируем), нет "void main()", нет выхода с именем
/// out_color_name. Рендер продолжает работать по прежнему пути.
bool wrap_fragment_shader_with_tonemap(
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    const std::string& tonemap_glsl,
    const char* out_color_name = "outColor"
);

#endif // GRAPHICS_SHADER_FUNCS_H
