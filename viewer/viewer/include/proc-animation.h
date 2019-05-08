#ifndef     PROC_ANIMATION_H
#define     PROC_ANIMATION_H

#include    <osg/MatrixTransform>
#include    "math-funcs.h"
#include    "config-reader.h"
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct key_point_t
{
    float param;
    float value;

    key_point_t()
        : param(0.0f)
        , value(0.0f)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcAnimation
{
public:

    ProcAnimation(osg::MatrixTransform *transform);

    virtual ~ProcAnimation();

    void step(float t, float dt);

    void setName(const std::string &name);

    std::string getName() const;

    bool load(const std::string &path);

    bool load(ConfigReader &cfg);

    void setPosition(float pos);

    size_t getSignalID() const;

protected:

    osg::MatrixTransform    *transform;
    std::string             name;

    float                   pos;
    float                   duration;
    float                   precision;

    size_t                  signal_id;

    std::vector<key_point_t> keypoints;

    virtual bool load_config(ConfigReader &cfg) = 0;

    virtual void anim_step(float t, float dt) = 0;

    float interpolate(float value);

private:

    bool loadKeyPoints(ConfigReader &cfg);

    key_point_t findBeginKeyPoint(float value, size_t &next_idx);
};

#endif // PROC_ANIMATION_H
