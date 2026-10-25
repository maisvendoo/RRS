#include "ProcParticlesAnimation.h"

#include "CfgReader.h"
#include "filesystem.h"

#include <vsg/core/Array.h>
#include <vsg/io/read.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/nodes/StateGroup.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcParticlesAnimation::ProcParticlesAnimation(vsg::ref_ptr<vsg::MatrixTransform> transform)
    : Inherit()
    , transform_node(transform)
{
    matrix = transform->matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform)
{
    transform_node = transform;
    matrix = transform->matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcParticlesAnimation::load_config(CfgReader& cfg)
{
    QString sec_name = "ParticlesAnimation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
    {
        signal_id = tmp_int;
    }

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
    {
        duration = tmp_dbl;
    }

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    params.speed_signal_id = -1;
    cfg.getInt(sec_name, "SpeedSignalID", params.speed_signal_id);

    QString tmp_qstr;
    if (cfg.getString(sec_name, "Texture", tmp_qstr))
    {
        params.texture = tmp_qstr.toStdString();
    }

    cfg.getDouble(sec_name, "Size", tmp_dbl);
    params.size = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "SizeVariation", tmp_dbl);
    params.size_variation = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "SpawnRate", tmp_dbl);
    params.spawn_rate = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "LifeTime", tmp_dbl);
    params.life_time = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "Speed", tmp_dbl);
    params.speed = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "SpeedVariation", tmp_dbl);
    params.speed_variation = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "Gravity", tmp_dbl);
    params.gravity = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "Spread", tmp_dbl);
    params.spread = static_cast<float>(tmp_dbl);

    cfg.getDouble(sec_name, "Drag", tmp_dbl);
    params.drag = static_cast<float>(tmp_dbl);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::initStateGroup()
{
    if (state_group != nullptr)
        return;

    FileSystem& fs = FileSystem::getInstance();

    std::string shaders_dir = fs.getDataDir() + fs.separator() + "shaders" + fs.separator();
    std::string vert_path = shaders_dir + "particle.vert";
    std::string frag_path = shaders_dir + "particle.frag";

    auto vert = vsg::read_cast<vsg::ShaderStage>(vert_path);
    auto frag = vsg::read_cast<vsg::ShaderStage>(frag_path);

    auto cbs = vsg::ColorBlendState::create();
    cbs->configureAttachments(true);

    auto dss = vsg::DepthStencilState::create();
    dss->depthWriteEnable = VK_FALSE;

    auto rs = vsg::RasterizationState::create();

    vsg::GraphicsPipelineStates pipe_states = { cbs, dss, rs };

    auto layout = vsg::PipelineLayout::create(
        vsg::DescriptorSetLayouts{},
        vsg::PushConstantRanges{}
    );

    auto pipeline = vsg::GraphicsPipeline::create(layout, vsg::ShaderStages{vert, frag}, pipe_states);
    auto bind_pipeline = vsg::BindGraphicsPipeline::create(pipeline);

    state_group = vsg::StateGroup::create();
    state_group->add(bind_pipeline);

    // Три arrays: vec3 (позиция), vec3 (size, alpha, 0), vec2 (uv)
    uint32_t max_v = static_cast<uint32_t>(MAX_PARTICLES) * 4;
    positions_array = vsg::vec3Array::create(max_v);
    normal_array = vsg::vec3Array::create(max_v);
    texcoord_array = vsg::vec2Array::create(max_v);
    index_array = vsg::ushortArray::create(static_cast<uint32_t>(MAX_PARTICLES) * 6);

    // Индексы — один раз
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        int base = i * 4;
        int ibase = i * 6;
        (*index_array)[ibase + 0] = static_cast<uint16_t>(base + 0);
        (*index_array)[ibase + 1] = static_cast<uint16_t>(base + 1);
        (*index_array)[ibase + 2] = static_cast<uint16_t>(base + 2);
        (*index_array)[ibase + 3] = static_cast<uint16_t>(base + 2);
        (*index_array)[ibase + 4] = static_cast<uint16_t>(base + 3);
        (*index_array)[ibase + 5] = static_cast<uint16_t>(base + 0);
    }

    vertex_index_draw = vsg::VertexIndexDraw::create();
    vertex_index_draw->assignArrays({positions_array, normal_array, texcoord_array});
    vertex_index_draw->assignIndices(index_array);
    vertex_index_draw->indexCount = 0;

    state_group->addChild(vertex_index_draw);

    // Добавляем StateGroup как child к ноде эмиттера
    if (transform_node != nullptr)
    {
        transform_node->addChild(state_group);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::update(float current_signal)
{
    emission_rate = params.spawn_rate * current_signal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::anim_step(float t, float dt)
{
    update_particles(t, dt);
    update_vertex_buffer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::update_particles(float t, float dt)
{
    // Обновляем снос от скорости ПЕ
    if (params.speed_signal_id >= 0 && server_signals &&
        params.speed_signal_id < static_cast<int>(server_signals->size()))
    {
        float speed_kmh = (*server_signals)[params.speed_signal_id];
        wind_velocity.x = -speed_kmh * 0.28f;
    }

    // Позиция эмиттера — локальная (StateGroup дочерний эмиттера)
    vsg::vec3 emitter_pos = {0.0f, 0.0f, 0.0f};

    // Спавним новые частицы
    accumulator += emission_rate * dt;
    while (accumulator >= 1.0f && particles.size() < MAX_PARTICLES)
    {
        Particle p;
        p.position = emitter_pos;

        float angle = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * params.spread;
        float angley = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * params.spread * 0.3f;

        float spd = params.speed * (1.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * params.speed_variation);
        p.velocity = {
            wind_velocity.x + sinf(angle) * spd,
            spd + wind_velocity.y,
            sinf(angley) * spd * 0.3f + wind_velocity.z
        };

        p.life = params.life_time * (1.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * params.size_variation);
        p.max_life = p.life;
        p.size = params.size * (1.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * params.size_variation);
        p.alpha = 1.0f;
        particles.push_back(p);
        accumulator -= 1.0f;
    }

    // Обновляем позиции частиц (движение в мировых координатах)
    for (auto& p : particles)
    {
        p.velocity.y -= params.gravity * dt;
        p.velocity *= params.drag;
        p.position += p.velocity * dt;
        p.life -= dt;
        p.alpha = std::clamp(p.life / p.max_life, 0.0f, 1.0f);
    }

    // Удаляем мёртвые
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcParticlesAnimation::update_vertex_buffer()
{
    if (vertex_index_draw == nullptr || positions_array == nullptr)
        return;

    uint32_t count = static_cast<uint32_t>(particles.size());
    uint32_t num_vertices = count * 4;

    for (uint32_t i = 0; i < count; ++i)
    {
        const auto& p = particles[i];
        uint32_t base = i * 4;

        for (uint32_t j = 0; j < 4; ++j)
        {
            uint32_t idx = base + j;
            float uv_x = (j == 1 || j == 2) ? 1.0f : 0.0f;
            float uv_y = (j == 2 || j == 3) ? 1.0f : 0.0f;

            (*positions_array)[idx] = vsg::vec3(p.position.x, p.position.y, p.position.z);
            (*normal_array)[idx] = vsg::vec3(p.size, p.alpha, 0.0f);
            (*texcoord_array)[idx] = vsg::vec2(uv_x, uv_y);
        }
    }

    vertex_index_draw->indexCount = num_vertices / 4 * 6;
}