#include    "analog-translation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogTranslation::AnalogTranslation(osg::MatrixTransform *transform)
    : ProcAnimation(transform)
    , min_motion(0.0f)
    , max_motion(0.0)
    , motion(0.0)
    , cur_pos(0.0)
    , axis(osg::Vec3())
    , matrix(transform->getMatrix())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogTranslation::~AnalogTranslation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogTranslation::anim_step(float t, float dt)
{
    (void) t;

    cur_pos += (pos - cur_pos) * duration * dt;

    motion = interpolate(cur_pos);

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnalogTranslation::load_config(ConfigReader &cfg)
{
    std::string secName = "AnalogRotation";

    cfg.getValue(secName, "SignalID", signal_id);
    cfg.getValue(secName, "MinAngle", min_motion);
    cfg.getValue(secName, "MaxAngle", max_motion);
    cfg.getValue(secName, "InitAngle", motion);
    cfg.getValue(secName, "Duration", duration);

    std::string tmp;
    cfg.getValue(secName, "Axis", tmp);

    std::istringstream ss(tmp);

    ss >> axis.x() >> axis.y() >> axis.z();

    cfg.getValue(secName, "Precision", precision);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogTranslation::update()
{
    if (keypoints.size() == 0)
        return;

    osg::Matrix translate = osg::Matrixf::translate(axis * motion);
    transform->setMatrix(translate * matrix);
}
