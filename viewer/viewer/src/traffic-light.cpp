#include	<traffic-light.h>
#include    <QBuffer>
#include    <anim-transform-visitor.h>


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::TrafficLight(QObject *parent) : QObject(parent)
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::~TrafficLight()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> conn_name;
    stream >> signal_dir;
    stream >> is_busy;
    stream >> letter;
    stream >> signal_model;

    for (size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }

    stream >> pos.x() >> pos.y() >> pos.z();
    stream >> orth.x() >> orth.y() >> orth.z();
    stream >> right.x() >> right.y() >> right.z();
    stream >> up.x() >> up.y() >> up.z();    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::update()
{
    if (lens_state != old_lens_state)
    {
        for (auto animation = animations.begin(); animation != animations.end(); ++animation)
        {
            ProcAnimation *anim = animation.value();
            anim->setPosition(lens_state[animation.value()->getSignalID()]);
        }

        old_lens_state = lens_state;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::load_animations(const std::string &animations_dir)
{
    AnimTransformVisitor atv(&animations, animations_dir);
    atv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    node->accept(atv);
}

