#include    "analog-rotation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogRotation::AnalogRotation(osg::MatrixTransform *transform) : ProcAnimation (transform)
    , min_angle(0.0f)
    , max_angle(osg::PIf)
    , angle(0.0f)
    , axis(osg::Vec3(0.0, 0.0, 1.0))
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

    float cur_pos = (angle - min_angle) / (max_angle - min_angle);

    angle += duration * dt * dead_zone(pos - cur_pos, -precision / 2.0f, precision / 2.0f);

    osg::Matrix matrix = transform->getMatrix();
    matrix *= osg::Matrixf::rotate(angle * osg::PIf / 180.0f, axis);
    transform->setMatrix(matrix);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnalogRotation::load_config(ConfigReader &cfg)
{
    std::string secName = "AnalogRotation";

    cfg.getValue(secName, "SignalID", signal_id);
    cfg.getValue(secName, "MinAngle", min_angle);
    cfg.getValue(secName, "MaxAngle", max_angle);
    cfg.getValue(secName, "InitAngle", angle);
    cfg.getValue(secName, "Duration", duration);

    std::string tmp;
    cfg.getValue(secName, "Axis", tmp);

    std::istringstream ss(tmp);

    ss >> axis.x() >> axis.y() >> axis.z();

    cfg.getValue(secName, "Precision", precision);

    return true;
}
