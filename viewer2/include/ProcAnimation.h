#ifndef PROC_ANIMATION_H
#define PROC_ANIMATION_H

#include <cstddef>
#include <string>
#include <vector>

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcAnimation : public vsg::Inherit<vsg::Object, ProcAnimation>
{
public:
    ProcAnimation() = default;

    virtual ~ProcAnimation() = default;

    void step(float t, float dt);

    bool load(CfgReader& cfg);

    void setPosition(float pos);

    std::size_t getSignalID() const;

public:
    std::string name = "";

protected:
    struct key_point_t
    {
        float param = 0.0f;
        float value = 0.0f;
    };

    float pos = 0.0f;
    float duration = 1.0f;

    std::size_t signal_id = 0;
    bool is_fixed_signal = false;
    float fixed_signal = 0.0f;

    std::vector<key_point_t> keypoints;

    virtual bool load_config(CfgReader& cfg) = 0;

    virtual void anim_step(float t, float dt) = 0;

    float interpolate(float value);

private:
    bool loadKeyPoints(CfgReader& cfg);

    key_point_t findBeginKeypoint(float value, std::size_t& next_idx);
};

#endif // PROC_ANIMATION_H
