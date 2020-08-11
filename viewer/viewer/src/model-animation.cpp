#include    "model-animation.h"

#include    "model-animation-visitor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelAnimation::ModelAnimation(osg::Node *model, const std::string &name)
    : ProcAnimation(name)
{
    ModelAnimationVisitor mav(&parts, name);
    mav.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    model->accept(mav);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelAnimation::~ModelAnimation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelAnimation::anim_step(float t, float dt)
{
    for (auto part : parts)
    {
        part->setRefPosition(static_cast<double>(pos));
        part->step(static_cast<double>(t), static_cast<double>(dt));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ModelAnimation::load_config(ConfigReader &cfg)
{
    std::string secName = "ModelAnimation";

    cfg.getValue(secName, "SignalID", signal_id);
    is_fixed_signal = cfg.getValue(secName, "FixedSignal", fixed_signal);
    double lastTime = 0.0;

    if (cfg.getValue(secName, "Time", lastTime))
    {
        if (parts.size() > 0)
        {
            for (auto part: parts)
            {
                part->setLastTime(lastTime);
            }
        }
    }

    return true;
}
