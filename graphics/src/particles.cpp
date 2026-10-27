#include "graphics/particles.h"

#include "Journal.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/commands/Command.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/maths/mat4.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/PushConstants.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/vk/CommandBuffer.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

namespace
{

//------------------------------------------------------------------------------
// GLSL: вершинный шейдер частицы. Каждая частица — 6 вершин (два
// треугольника квада), индекс частицы = gl_VertexIndex / 6. Биллборд
// строится в пространстве вида: центр из атрибута переводится
// modelView, углы квада смещаются по осям right/up пространства вида
// (первый/второй столбцы матрицы проекции не нужны — смещение
// выполняется ДО проекции). Матрицы приходят push-константами
//------------------------------------------------------------------------------
const char* particle_vertex_source()
{
    return
        "#version 450\n"
        "\n"
        "layout(location = 0) in vec4 in_position_size;\n"
        "layout(location = 1) in vec4 in_color_alpha;\n"
        "\n"
        "layout(push_constant) uniform PC\n"
        "{\n"
        "    mat4 projection;\n"
        "    mat4 modelView;\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) out vec4 v_color;\n"
        "layout(location = 1) out vec2 v_uv;\n"
        "\n"
        "const vec2 corners[6] = vec2[6](\n"
        "    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),\n"
        "    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0));\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec2 corner = corners[gl_VertexIndex % 6];\n"
        "\n"
        "    vec4 center_view = pc.modelView * vec4(in_position_size.xyz, 1.0);\n"
        "    vec4 corner_view = center_view +\n"
        "        vec4(corner.x * in_position_size.w,\n"
        "             corner.y * in_position_size.w, 0.0, 0.0);\n"
        "\n"
        "    gl_Position = pc.projection * corner_view;\n"
        "\n"
        "    v_color = in_color_alpha;\n"
        "    v_uv = corner * 0.5 + 0.5;\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: фрагментный шейдер — мягкий круглый спрайт без текстуры
// (радиальный градиент по uv). Глубина не пишется (полупрозрачный
// проход), альфа срезается ниже порога как в standard_flat_shaded.frag
//------------------------------------------------------------------------------
std::string particle_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(location = 0) in vec4 v_color;\n"
        "layout(location = 1) in vec2 v_uv;\n"
        "\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float dist = length(v_uv - vec2(0.5)) * 2.0;\n"
        "\n"
        "    float edge = clamp(1.0 - dist, 0.0, 1.0);\n"
        "    edge *= edge;   // мягкий край спрайта\n"
        "\n"
        "    float alpha = v_color.a * edge;\n"
        "\n"
        "    if (alpha < 0.01)\n"
        "    {\n"
        "        discard;\n"
        "    }\n"
        "\n"
        "    outColor = vec4(v_color.rgb, alpha);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// Push-константы матриц частиц: projection/modelView читаются из
// vsg::Camera В МОМЕНТ записи кадра (как DynamicPushConstants в
// postprocess.cpp), поэтому билборды всегда смотрят в актуальную камеру
// и отдельный механизм обновления буферов не нужен
//------------------------------------------------------------------------------
class ParticleMatrices final : public vsg::Inherit<vsg::Command, ParticleMatrices>
{
public:
    ParticleMatrices(vsg::ref_ptr<vsg::PipelineLayout> layout,
                     vsg::ref_ptr<vsg::Camera> camera) :
        layout_(std::move(layout)),
        camera_(std::move(camera))
    {
    }

    void record(vsg::CommandBuffer& commandBuffer) const override
    {
        if (!layout_ || !camera_ || !camera_->projectionMatrix ||
            !camera_->viewMatrix)
        {
            return;
        }

        // double-матрицы камеры -> float для push-констант
        const vsg::mat4 projection(camera_->projectionMatrix->transform());
        const vsg::mat4 model_view(camera_->viewMatrix->transform());

        struct
        {
            vsg::mat4 projection;
            vsg::mat4 model_view;
        } data{projection, model_view};

        vkCmdPushConstants(commandBuffer,
                           layout_->vk(commandBuffer.deviceID),
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(data), &data);
    }

private:
    vsg::ref_ptr<vsg::PipelineLayout> layout_;
    vsg::ref_ptr<vsg::Camera> camera_;
};

} // namespace

namespace graphics
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ParticleSystem::ParticleSystem(std::size_t max_particles,
                               vsg::ref_ptr<vsg::Camera> camera) :
    max_particles_(max_particles ? max_particles : 1),
    particles_(max_particles ? max_particles : 1)
{
    if (max_particles == 0)
    {
        Journal::instance()->warning(
            "Particles: zero pool size, forced to 1");
    }

    if (!camera)
    {
        Journal::instance()->warning(
            "Particles: no camera, system is not created");
        return;
    }

    // Динамические массивы частиц: DYNAMIC_DATA + dirty() после каждого
    // шага — их переносит в GPU vsg::TransferTask (назначается
    // viewer->compile(), см. комментарий vsg/app/TransferTask.h)
    positions_ = vsg::vec4Array::create(
        static_cast<uint32_t>(max_particles_));
    colors_ = vsg::vec4Array::create(
        static_cast<uint32_t>(max_particles_));

    positions_->properties.dataVariance = vsg::DYNAMIC_DATA;
    colors_->properties.dataVariance = vsg::DYNAMIC_DATA;

    std::fill(positions_->begin(), positions_->end(),
              vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    std::fill(colors_->begin(), colors_->end(),
              vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Шейдеры из GLSL-строк (компилируются glslang'ом при компиляции
    // пайплайна, как проходы PostProcessChain)
    auto vertex_shader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_VERTEX_BIT, "main", particle_vertex_source());
    auto fragment_shader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_FRAGMENT_BIT, "main", particle_fragment_source());

    // Push-константы: две mat4 = 128 байт (предел min maxPushConstantsSize)
    const vsg::PushConstantRanges push_ranges{
        VkPushConstantRange{VK_SHADER_STAGE_VERTEX_BIT, 0,
                            2 * sizeof(vsg::mat4)}
    };

    const auto pipeline_layout = vsg::PipelineLayout::create(
        vsg::DescriptorSetLayouts{}, push_ranges);

    // Вершинные привязки: два vec4-атрибута (позиция+размер, цвет+альфа)
    const vsg::VertexInputState::Bindings bindings{
        VkVertexInputBindingDescription{
            0, sizeof(vsg::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{
            1, sizeof(vsg::vec4), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    const vsg::VertexInputState::Attributes attributes{
        VkVertexInputAttributeDescription{
            0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        VkVertexInputAttributeDescription{
            1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}
    };

    // Полупрозрачный проход: alpha-blending, глубина тестируется
    // (частицы прячутся за геометрией), но не пишется. Cull выключен —
    // биллборд генерируется ориентированным на камеру
    auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;

    auto depth_state = vsg::DepthStencilState::create();
    depth_state->depthTestEnable = VK_TRUE;
    depth_state->depthWriteEnable = VK_FALSE;

    auto blend_state = vsg::ColorBlendState::create();
    blend_state->attachments = vsg::ColorBlendState::ColorBlendAttachments{
        {
            true,                                   // blending enabled
            VK_BLEND_FACTOR_SRC_ALPHA,              // srcColorBlendFactor
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // dstColorBlendFactor
            VK_BLEND_OP_ADD,                        // colorBlendOp
            VK_BLEND_FACTOR_ONE,                    // srcAlphaBlendFactor
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // dstAlphaBlendFactor
            VK_BLEND_OP_ADD,                        // alphaBlendOp
            VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT
        }
    };

    // Сэмплы MSAA не задаём явно (VK_SAMPLE_COUNT_1_BIT по умолчанию):
    // при компиляции внутри RenderGraph окна состояние переопределяется
    // сэмплами фреймбуфера (context.overridePipelineStates), как у всех
    // пайплайнов сцены
    const vsg::GraphicsPipelineStates pipeline_states{
        vsg::VertexInputState::create(bindings, attributes),
        vsg::InputAssemblyState::create(),      // TRIANGLE_LIST по умолчанию
        rasterization_state,
        vsg::MultisampleState::create(),
        blend_state,
        depth_state
    };

    const auto pipeline = vsg::GraphicsPipeline::create(
        pipeline_layout,
        vsg::ShaderStages{vertex_shader, fragment_shader},
        pipeline_states);

    // Отрисовка: 6 вершин на частицу, счёт фиксированный — мёртвые
    // частицы являются вырожденными квадами (размер 0)
    auto draw = vsg::VertexDraw::create();
    draw->firstBinding = 0;
    draw->assignArrays(vsg::DataList{positions_, colors_});
    draw->vertexCount = static_cast<uint32_t>(max_particles_) * 6;
    draw->instanceCount = 1;
    draw->firstVertex = 0;
    draw->firstInstance = 0;

    auto state_group = vsg::StateGroup::create();
    state_group->add(vsg::BindGraphicsPipeline::create(pipeline));
    state_group->addChild(ParticleMatrices::create(pipeline_layout, camera));
    state_group->addChild(draw);

    node_ = state_group;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> ParticleSystem::getNode() const
{
    return node_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ParticleSystem::emitParticles(const Spawn& spawn)
{
    if (!node_ || spawn.lifetime <= 0.0f)
    {
        return false;
    }

    // Кольцевой поиск свободного слота от последнего заспавненного
    for (std::size_t probe = 0; probe < max_particles_; ++probe)
    {
        emit_cursor_ = (emit_cursor_ + 1) % max_particles_;
        Particle& candidate = particles_[emit_cursor_];

        if (candidate.age >= candidate.lifetime)
        {
            candidate.position = spawn.position;
            candidate.velocity = spawn.velocity;
            candidate.color = spawn.color;
            candidate.alpha = spawn.alpha;
            candidate.size_begin = spawn.size_begin;
            candidate.size_end = spawn.size_end;
            candidate.lifetime = spawn.lifetime;
            candidate.age = 0.0f;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ParticleSystem::step(double dt)
{
    if (!node_ || dt <= 0.0)
    {
        return;
    }

    const float dtf = static_cast<float>(dt);
    std::size_t alive = 0;

    for (std::size_t i = 0; i < max_particles_; ++i)
    {
        Particle& p = particles_[i];

        if (p.age >= p.lifetime)
        {
            // Мёртвый слот — вырожденный квад
            positions_->at(i) = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors_->at(i) = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            continue;
        }

        p.age += dtf;
        p.position += p.velocity * dtf;

        if (p.age >= p.lifetime)
        {
            positions_->at(i) = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors_->at(i) = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            continue;
        }

        ++alive;

        // Доля прожитой жизни 0..1: рост размера и fade in/out по ней
        const float k = p.age / p.lifetime;

        const float size = p.size_begin +
            (p.size_end - p.size_begin) * k;

        // Плавное появление (первые 15% жизни) и растворение
        // (последние 40%) — резких вспышек/кликов альфы нет
        const float fade_in = std::min(k / 0.15f, 1.0f);
        const float fade_out = std::min((1.0f - k) / 0.4f, 1.0f);
        const float alpha = p.alpha * std::min(fade_in, fade_out);

        positions_->at(i) = vsg::vec4(p.position, size);
        colors_->at(i) = vsg::vec4(p.color, alpha);
    }

    alive_count_ = alive;

    // Помечаем массивы изменёнными: TransferTask перенесёт их в GPU
    // перед обходом записи текущего кадра
    positions_->dirty();
    colors_->dirty();
}

} // namespace graphics
