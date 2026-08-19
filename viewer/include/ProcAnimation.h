#pragma once
#ifndef PROC_ANIMATION_H
#define PROC_ANIMATION_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <string>
#include <vector>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcAnimation : public vsg::Inherit<vsg::Object, ProcAnimation>
{
public:
    ProcAnimation() = default;

    virtual ~ProcAnimation() noexcept = default;

    void step(float t, float dt);

    bool load(CfgReader& cfg);

    void setSignals(std::vector<float>* in_signals);

    virtual std::size_t getSignalID() const;

public:
    std::string name = "";

protected:
    std::int32_t signal_id = -1;
    bool is_fixed_signal = false;
    float cur_signal = 0.0f;
    float duration = 1.0f;

    struct key_point_t
    {
        float param = 0.0f;
        float value = 0.0f;
    };

    std::vector<key_point_t> keypoints;

    std::vector<float>* server_signals = nullptr;

    virtual bool load_config(CfgReader& cfg) = 0;

    virtual void anim_step(float t, float dt);

    virtual void update(float current_signal) = 0;

    float interpolate(float value);

private:
    bool loadKeyPoints(CfgReader& cfg);

    void findBeginKeypoint(float param, key_point_t& begin_point, key_point_t& next_point);
};

#endif // PROC_ANIMATION_H
