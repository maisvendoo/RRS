#ifndef     PROC_ANIMATION_H
#define     PROC_ANIMATION_H

#include    <osg/MatrixTransform>
#include    "math-funcs.h"
#include    "config-reader.h"
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcAnimation
{
public:

    ProcAnimation()
        : pos(0.0)
        , duration(0.0)
        , signal_id(0)
        , transform(nullptr)
        , name("")
    {

    }

    ProcAnimation(const std::string &name)
        : pos(0.0)
        , duration(0.0)
        , signal_id(0)
        , transform(nullptr)
        , name(name)
    {

    }

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

    float                   pos;
    float                   duration;

    size_t                  signal_id;

    osg::MatrixTransform    *transform;
    std::string             name;

    bool                    is_fixed_signal;
    float                   fixed_signal;

    std::vector<key_point_t> keypoints;

    virtual bool load_config(ConfigReader &cfg) = 0;

    virtual void anim_step(float t, float dt) = 0;

    float interpolate(float value);

private:

    bool loadKeyPoints(ConfigReader &cfg);

    key_point_t findBeginKeyPoint(float value, size_t &next_idx);
};

#endif // PROC_ANIMATION_H
