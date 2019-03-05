#ifndef     PROC_ANIMATION_H
#define     PROC_ANIMATION_H

#include    <osg/MatrixTransform>
#include    "math-funcs.h"
#include    "config-reader.h"
#include    <strstream>

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

    virtual bool load_config(ConfigReader &cfg) = 0;

    virtual void anim_step(float t, float dt) = 0;
};

#endif // PROC_ANIMATION_H
