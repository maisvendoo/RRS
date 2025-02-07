#ifndef PROC_ANIMATION_H
#define PROC_ANIMATION_H

#include <string>
#include <vector>

#include <cstddef>

class ConfigReader;

namespace vsg
{
    class MatrixTransform;
}

class ProcAnimation
{
public:
    ProcAnimation() = default;
    ProcAnimation(const std::string& name);
    ProcAnimation(vsg::MatrixTransform* transform);

    virtual ~ProcAnimation() = default;

    void step(float t, float dt);

    std::string name = "";

    bool load(const std::string& path);

    bool load(ConfigReader& cfg);

    void setPosition(float pos);

    std::size_t getSignalID() const;

protected:
    struct key_point_t
    {
        float param = 0.0f;
        float value = 0.0f;
    };

    float pos = 0.0f;
    float duration = 0.0f;

    std::size_t signal_id = 0;

    vsg::MatrixTransform* transform = nullptr;

    bool is_fixed_signal;
    float fixed_signal;

    std::vector<key_point_t> keypoints;

    virtual bool load_config(ConfigReader& cfg) = 0;

    virtual void anim_step(float t, float dt) = 0;

    float interpolate(float value);

private:
    bool loadKeyPoints(ConfigReader& cfg);

    key_point_t findBeginKeypoint(float value, std::size_t& next_idx);
};

#endif // PROC_ANIMATION_H
