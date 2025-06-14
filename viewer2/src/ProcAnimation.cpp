#include "ProcAnimation.h"

#include "CfgReader.h"

#include <vsg/nodes/MatrixTransform.h>

#include <cstddef>
#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::step(float t, float dt)
{
    anim_step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::load(CfgReader &cfg)
{
    loadKeyPoints(cfg);
    return load_config(cfg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::setSignals(std::vector<float> *in_signals)
{
    server_signals = in_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::size_t ProcAnimation::getSignalID() const
{
    return signal_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::load_config(CfgReader &cfg)
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::anim_step([[maybe_unused]] float t, float dt)
{
    if (is_fixed_signal)
        return;

    float server_signal = 0.0f;
    if (server_signals && (signal_id < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id];
    }

    float delta = server_signal - cur_signal;
    if (abs(delta) > 1e-5f)
    {
        cur_signal += delta * fmin(duration * dt, 1.0f);
        update(cur_signal);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::update([[maybe_unused]] float current_signal)
{
    return;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ProcAnimation::interpolate(float param)
{
    if (keypoints.size() <= 1)
    {
        return 0.0f;
    }

    std::size_t next_idx = 0;
    key_point_t begin_point = findBeginKeypoint(param, next_idx);
    key_point_t next_point = keypoints.at(next_idx);

    float range = next_point.param - begin_point.param;

    if (range < 1e-6f)
    {
        return 0.0f;
    }

    float value = begin_point.value + (param - begin_point.param) * (next_point.value - begin_point.value) / range;
    return value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::loadKeyPoints(CfgReader &cfg)
{
    QDomNode config_section = cfg.getFirstSection("KeyPoint");
    while (!config_section.isNull())
    {
        key_point_t keypoint;

        double tmp = 0.0;
        if (cfg.getDouble(config_section, "Param", tmp))
            keypoint.param = tmp;

        tmp = 0.0;
        if (cfg.getDouble(config_section, "Value", tmp))
            keypoint.value = tmp;

        keypoints.emplace_back(std::move(keypoint));
        config_section = cfg.getNextSection();
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcAnimation::key_point_t ProcAnimation::findBeginKeypoint(float param, std::size_t& next_idx)
{
    key_point_t key_point;

    if (keypoints.empty())
    {
        return key_point;
    }

    std::size_t left_idx = 0;
    std::size_t right_idx = keypoints.size() - 1;
    std::size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        key_point = keypoints.at(idx);
        if (param <= key_point.param)
        {
            right_idx = idx;
        }
        else
        {
            left_idx = idx;
        }

        idx = (left_idx + right_idx) / 2;
    }

    key_point = keypoints.at(idx);
    next_idx = idx + 1;

    return key_point;
}
