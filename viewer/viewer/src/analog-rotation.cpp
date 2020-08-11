#include    "analog-rotation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogRotation::AnalogRotation(osg::MatrixTransform *transform) : ProcAnimation (transform)
    , min_angle(0.0f)
    , max_angle(osg::PIf)
    , angle(0.0f)
    , cur_pos(0.0f)
    , infinity(false)
    , axis(osg::Vec3(0.0, 0.0, 1.0))
    , matrix(transform->getMatrix())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogRotation::~AnalogRotation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogRotation::anim_step(float t, float dt)
{
    (void) t;

    cur_pos += (pos - cur_pos) * duration * dt;

    angle = interpolate(cur_pos);

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnalogRotation::load_config(ConfigReader &cfg)
{
    std::string secName = "AnalogRotation";

    cfg.getValue(secName, "SignalID", signal_id);    
    cfg.getValue(secName, "Duration", duration);
    is_fixed_signal = cfg.getValue(secName, "FixedSignal", fixed_signal);

    int inf = 0;
    cfg.getValue(secName, "Infinity", inf);

    infinity = static_cast<bool>(inf);

    std::string tmp;
    cfg.getValue(secName, "Axis", tmp);

    std::istringstream ss(tmp);

    ss >> axis.x() >> axis.y() >> axis.z();    

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogRotation::update()
{
    if (keypoints.size() == 0)
        return;

    if (!infinity)
        angle = cut(angle, (*keypoints.begin()).value, (*(keypoints.end() - 1)).value);

    osg::Matrix rotate = osg::Matrixf::rotate(angle * osg::PIf / 180.0f, axis);
    transform->setMatrix(rotate * matrix);
}
