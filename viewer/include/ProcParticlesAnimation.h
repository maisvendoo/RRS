#pragma once
#ifndef PROC_PARTICLES_ANIMATION_H
#define PROC_PARTICLES_ANIMATION_H

#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

class CfgReader;

namespace vsg
{
    class Group;
    class StateGroup;
    class VertexIndexDraw;
    class VertexBuffer;
    class MatrixTransform;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcParticlesAnimation final : public vsg::Inherit<ProcAnimation, ProcParticlesAnimation>
{
public:
    explicit ProcParticlesAnimation(vsg::ref_ptr<vsg::MatrixTransform> transform);

    void setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform);

    vsg::ref_ptr<vsg::StateGroup> getStateGroup() const
    {
        return state_group;
    }

    void initStateGroup();

private:
    // Параметры эмиттера из конфига
    struct
    {
        std::string texture;
        float size = 0.5f;
        float size_variation = 0.2f;
        float spawn_rate = 20.0f;
        float life_time = 2.5f;
        float speed = 1.5f;
        float speed_variation = 0.4f;
        float gravity = 0.3f;
        float spread = 0.4f;
        float drag = 0.97f;
        int speed_signal_id = -1;
    } params;

    // Частица
    struct Particle
    {
        vsg::vec3 position;
        vsg::vec3 velocity;
        float life;
        float max_life;
        float size;
        float alpha;
    };
    std::vector<Particle> particles;
    static constexpr int MAX_PARTICLES = 500;

    float accumulator = 0.0f;
    float emission_rate = 0.0f;
    vsg::vec3 wind_velocity = {0.0f, 0.0f, 0.0f};

    // VSG-объекты для рендера
    vsg::ref_ptr<vsg::StateGroup> state_group = nullptr;
    vsg::ref_ptr<vsg::VertexIndexDraw> vertex_index_draw = nullptr;

    vsg::ref_ptr<vsg::vec3Array> positions_array = nullptr;
    vsg::ref_ptr<vsg::vec3Array> normal_array = nullptr;
    vsg::ref_ptr<vsg::vec2Array> texcoord_array = nullptr;
    vsg::ref_ptr<vsg::ushortArray> index_array = nullptr;

    // 3D-нода эмиттера (Empty в модели)
    vsg::ref_ptr<vsg::MatrixTransform> transform_node = nullptr;
    vsg::dmat4 matrix;

    bool load_config(CfgReader &cfg) override;

    void update(float current_signal) override;

    void anim_step(float t, float dt) override;

    void update_particles(float t, float dt);

    void update_vertex_buffer();
};

#endif // PROC_PARTICLES_ANIMATION_H