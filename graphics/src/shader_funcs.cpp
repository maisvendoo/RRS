#include "graphics/shader_funcs.h"

#include "Journal.h"
#include "filesystem.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/state/ShaderModule.h>
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
        Journal::instance()->warning(QString("Failed to load shader %1").arg(shader_path.c_str()));
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

    // Встроенные варианты VSG ShaderSet НЕ очищаем: варианты должны
    // накапливаться, иначе теряются стандартные комбинацииdefines
    // (например, LIGHTING, HARD_SHADOWS) из create*ShaderSet

    Journal::instance()->info(QString{"Shader set %1 configured"}
        .arg(shader_set_name));
}

//------------------------------------------------------------------------------
// Имя, в которое переименовывается исходный main() фрагментного шейдера
// при обёртке тонмаппингом (должно совпадать с именем, вызываемым
// gfx::aces_tonemap_shader_fragment())
//------------------------------------------------------------------------------
static const char* TONEMAP_ORIGINAL_MAIN = "rrs_tonemap_original_main";

bool wrap_fragment_shader_with_tonemap(
    vsg::ref_ptr<vsg::ShaderStage> frag_shader,
    const std::string& tonemap_glsl,
    const char* out_color_name
)
{
    if (!frag_shader || !frag_shader->module)
    {
        Journal::instance()->warning(
            QString{"Tonemap: fragment shader is not loaded, skip injection"});
        return false;
    }

    const vsg::ref_ptr<vsg::ShaderModule> module = frag_shader->module;
    std::string& source = module->source;

    // Тонмаппинг инжектится только в GLSL-исходник: модуль с готовым
    // SPIR-V (или без исходника) модифицировать ненадёжно
    if (source.empty() || !module->code.empty())
    {
        Journal::instance()->warning(
            QString{"Tonemap: no editable GLSL source, skip injection"});
        return false;
    }

    // Повторная обёртка не нужна и опасна (нет исходного main())
    if (source.find(TONEMAP_ORIGINAL_MAIN) != std::string::npos)
    {
        return true;
    }

    // Оборачиваем только шейдеры со стандартным выходом цвета
    if (source.find(out_color_name) == std::string::npos)
    {
        Journal::instance()->warning(QString{
            "Tonemap: fragment output %1 not found, skip injection"}
            .arg(out_color_name));
        return false;
    }

    // Переименовываем исходный main(): все шейдеры проекта объявляют
    // его строго как "void main()" (data/shaders/*.frag)
    const std::string original_main{"void main()"};
    const std::size_t main_pos = source.find(original_main);
    if (main_pos == std::string::npos)
    {
        Journal::instance()->warning(
            QString{"Tonemap: original main() not found, skip injection"});
        return false;
    }

    source.replace(main_pos, original_main.length(),
                   std::string{"void "} + TONEMAP_ORIGINAL_MAIN + "()");

    // Дописываем ACES-функцию и новый main(), вызывающий исходный.
    // #include-директивы шейдера стоят до исходного main(), поэтому
    // дописывание в конец безопасно при любом порядке вставки include-ов
    source += "\n";
    source += tonemap_glsl;

    // На случай, если модуль уже был скомпилирован: чистим SPIR-V,
    // чтобы он пересобрался из изменённого GLSL при компиляции пайплайна
    module->code.clear();

    Journal::instance()->info(QString{
        "Tonemap: ACES tonemapping injected into fragment shader"});

    return true;
}
