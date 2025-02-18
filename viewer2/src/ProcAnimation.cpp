#include "ProcAnimation.h"

#include "ConfigReader.h"

#include <vsg/nodes/MatrixTransform.h>

#include <cstddef>
#include <string>
#include <vector>

ProcAnimation::ProcAnimation(const std::string& name)
    : name(name)
{
}

ProcAnimation::ProcAnimation(vsg::MatrixTransform* transform)
    : transform(transform)
{
}

void ProcAnimation::step(float t, float dt)
{
    anim_step(t, dt);
}

bool ProcAnimation::load(const std::string& path)
{
    ConfigReader cfg(path);
    return load_config(cfg);
}

bool ProcAnimation::load(ConfigReader& cfg)
{
    loadKeyPoints(cfg);
    return load_config(cfg);
}

void ProcAnimation::setPosition(float pos)
{
    if (is_fixed_signal)
    {
        this->pos = fixed_signal;
    }
    else
    {
        this->pos = pos;
    }
}

std::size_t ProcAnimation::getSignalID() const
{
    return signal_id;
}

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

bool ProcAnimation::loadKeyPoints(ConfigReader& cfg)
{
    auto config_section = cfg.getConfigSection();
    for (auto child : config_section.children())
    {
        std::string name = child.name();
        if (name == "KeyPoint")
        {
            key_point_t keypoint;
            cfg.setSection(child);
            cfg.getValue("Param", keypoint.param);
            cfg.getValue("Value", keypoint.value);
            keypoints.emplace_back(std::move(keypoint));
        }
    }

    return true;
}

ProcAnimation::key_point_t ProcAnimation::findBeginKeypoint(float param, std::size_t& next_idx)
{
    key_point_t key_point;

    if (keypoints.empty())
    {
        return key_point;
    }

    std::size_t left_idx = 0;
    std::size_t right_idx = keypoints.size() - 1;
    std::size_t idx = (left_idx + right_idx) * 0.5;

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

        idx = (left_idx + right_idx) * 0.5;
    }

    key_point = keypoints.at(idx);
    next_idx = idx + 1;

    return key_point;
}
