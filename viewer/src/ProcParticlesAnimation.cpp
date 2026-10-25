#include "ProcParticlesAnimation.h"

#include "CfgReader.h"
#include "filesystem.h"

#include <vsg/io/read.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/StateGroup.h>
#include <vsg/state/VertexBuffer.h>

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
void ProcParticlesAnimation::load_config(CfgReader& cfg)
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

    cfg.getInt(sec_name, "SpeedSignalID", params.speed_signal_id, -1);

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

    auto vert = vsg::read(vert_path);
    auto frag = vsg::read(frag_path);

    auto pipeline = vsg::GraphicsPipeline::create();
    pipeline->vertexShader = vert;
    pipeline->fragmentShader = frag;

    auto cbs = vsg::ColorBlendState::create();
    cbs->configureAttachments(true);

    auto dss = vsg::DepthStencilState::create();
    dss->depthWriteEnable = VK_FALSE;

    state_group = vsg::StateGroup::create();
    state_group->add(pipeline);
    state_group->add(cbs);
    state_group->add(dss);
    state_group->add(vsg::RasterizationState::create());

    vertex_buffer = vsg::VertexBuffer::create();

    vertex_index_draw = vsg::VertexIndexDraw::create();
    vertex_index_draw->buffer = vertex_buffer;

    state_group->add(vertex_index_draw);

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
void ProcParticlesAnimation::update_particles(float t, float dt)
{
    // Обновляем снос от скорости ПЕ
    if (params.speed_signal_id >= 0 && server_signals &&
        params.speed_signal_id < static_cast<int>(server_signals->size()))
    {
        float speed_kmh = (*server_signals)[params.speed_signal_id];
        wind_velocity.x = -speed_kmh * 0.28f;
    }

    // Позиция эмиттера в мировой системе координат
    vsg::vec3 emitter_pos = {0.0f, 0.0f, 0.0f};
    if (transform_node != nullptr)
    {
        vsg::dmat4 world = transform_node->getWorldMatrix();
        emitter_pos.x = static_cast<float>(world[3]);
        emitter_pos.y = static_cast<float>(world[7]);
        emitter_pos.z = static_cast<float>(world[11]);
    }

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
    if (vertex_buffer == nullptr || vertex_index_draw == nullptr)
        return;

    // 4 вершины на частицу (квад)
    size_t num_vertices = particles.size() * 4;
    vertex_buffer->resize(num_vertices);

    auto map = vertex_buffer->map();
    if (!map)
        return;

    for (size_t i = 0; i < particles.size(); ++i)
    {
        const auto& p = particles[i];
        size_t base = i * 4;

        for (int j = 0; j < 4; ++j)
        {
            size_t idx = base + j;
            float uv_x = (j == 1 || j == 2) ? 1.0f : 0.0f;
            float uv_y = (j == 2 || j == 3) ? 1.0f : 0.0f;

            map->set(idx, 0, p.position.x);
            map->set(idx, 1, p.position.y);
            map->set(idx, 2, p.position.z);
            map->set(idx, 3, p.size);
            map->set(idx, 4, p.alpha);
            map->set(idx, 5, uv_x);
            map->set(idx, 6, uv_y);
        }
    }

    map->unmap();
}