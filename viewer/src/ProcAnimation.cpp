#include "ProcAnimation.h"

#include "CfgReader.h"

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
void ProcAnimation::anim_step([[maybe_unused]] float t, float dt)
{
    if (is_fixed_signal)
    {
        return;
    }

    float server_signal = 0.0f;
    if (server_signals && (signal_id >= 0) && (static_cast<std::size_t>(signal_id) < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id];
    }

    const float delta = server_signal - cur_signal;
    if (std::abs(delta) > 1e-5f)
    {
        cur_signal += delta * std::min(duration * dt, 1.0f);
        update(cur_signal);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ProcAnimation::interpolate(float param)
{
    if (keypoints.size() <= 1)
    {
        if (keypoints.size() == 1)
            return keypoints.front().value;

        return 0.0f;
    }

    key_point_t begin_point;
    key_point_t next_point;
    findBeginKeypoint(param, begin_point, next_point);

    const float range = next_point.param - begin_point.param;

    if (range < 1e-6f)
    {
        return begin_point.value;
    }

    const float value = begin_point.value + (param - begin_point.param) * (next_point.value - begin_point.value) / range;
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
        {
            keypoint.param = tmp;
        }

        tmp = 0.0;
        if (cfg.getDouble(config_section, "Value", tmp))
        {
            keypoint.value = tmp;
        }

        keypoints.emplace_back(std::move(keypoint));
        config_section = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::findBeginKeypoint(float param, key_point_t& begin_point, key_point_t& next_point)
{
    std::size_t left_idx = 0;
    std::size_t right_idx = keypoints.size() - 1;
    std::size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        begin_point = keypoints.at(idx);
        if (param <= begin_point.param)
        {
            right_idx = idx;
        }
        else
        {
            left_idx = idx;
        }

        idx = (left_idx + right_idx) / 2;
    }

    if (idx == (keypoints.size() - 1))
    {
        begin_point = keypoints.at(idx - 1);
        next_point = keypoints.at(idx);
    }
    else
    {
        begin_point = keypoints.at(idx);
        next_point = keypoints.at(idx + 1);
    }
}
