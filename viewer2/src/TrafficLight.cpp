#include    <TrafficLight.h>
#include    <AnimTransformVisitor.h>
#include    <algorithm>
#include    <qbuffer.h>
#include    <qflags.h>
#include    <iostream>
#include <vsg/utils/PropagateDynamicObjects.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::TrafficLight()
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::deserialize(QByteArray& data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> connector_name;
    stream >> signal_dir;
    stream >> is_busy;
    stream >> letter;
    stream >> signal_model;

    for (std::size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }

    stream >> pos.x >> pos.y >> pos.z;
    stream >> orth.x >> orth.y >> orth.z;
    stream >> right.x >> right.y >> right.z;
    stream >> up.x >> up.y >> up.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getConnectorName() const
{
    return connector_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int TrafficLight::getSignalDirection() const
{
    return signal_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getLetter() const
{
    return letter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getModelName() const
{
    return signal_model;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const vsg::dvec3& TrafficLight::getPosition() const
{
    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const vsg::dvec3& TrafficLight::getOrth() const
{
    return orth;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const vsg::dvec3& TrafficLight::getRight() const
{
    return right;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const vsg::dvec3& TrafficLight::getUp() const
{
    return up;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::setNode(vsg::ref_ptr<vsg::MatrixTransform>& node)
{
    this->node = &node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::load_animations(const std::string& animations_dir, vsg::ref_ptr<vsg::Options> options)
{
    AnimTransformVisitor atv(&animations, animations_dir, *node, options);
    (*node)->accept(atv);

    for (auto animation : animations)
    {
        animation.second->setPosition(lens_state[animation.first]);
    }
    
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::step(float t, float dt)
{
    if (animations.empty())
    {
        return;
    }

    bool changed = (old_lens_state != lens_state);
    if (changed)
    {
        std::cout << "Updated signal "
                  << this->getConnectorName().toStdString()
                  << " | lens: "
                  << lens_state[0] << lens_state[1] << lens_state[2] << lens_state[3] << lens_state[4]
                  << std::endl;

        old_lens_state = lens_state;
    }

    for (auto animation : animations)
    {
        if (changed)
            animation.second->setPosition(lens_state[animation.first]);

        animation.second->step(t, dt);
    }
}
