#ifndef     MODEL_ANIMATION_H
#define     MODEL_ANIMATION_H

#include    "proc-animation.h"
#include    "model-parts-list.h"
#include    "model-part-animation.h"
#include    "config-reader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ModelAnimation : public ProcAnimation
{
public:

    ModelAnimation(osg::Node *model, const std::string &name);

    ~ModelAnimation();

private:

    model_parts_list_t  parts;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader &cfg);
};

#endif // MODELANIMATION_H
