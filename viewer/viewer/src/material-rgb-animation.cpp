#include    "material-rgb-animation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MaterialRGBAnimation::MaterialRGBAnimation(osg::Material *mat, osg::Drawable *drawable)
    : ProcAnimation()
    , mat(new osg::Material)
    , drawable(drawable)
    , stateset(new osg::StateSet(*drawable->getOrCreateStateSet(), osg::CopyOp::DEEP_COPY_STATESETS))
    , cur_pos_r(0.0f)
    , cur_pos_b(0.0f)
    , cur_pos_g(0.0f)
    , pos_r(0.0f)
    , pos_b(0.0f)
    , pos_g(0.0f)
    , color(mat->getDiffuse(osg::Material::FRONT_AND_BACK))
    , emission_color(osg::Vec4(0.0, 0.0, 0.0, 1.0))
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MaterialRGBAnimation::~MaterialRGBAnimation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MaterialRGBAnimation::anim_step(float t, float dt)
{
    (void) t;

    cur_pos_r += (pos_r - cur_pos_r) * duration * dt;
    cur_pos_g += (pos_g - cur_pos_g) * duration * dt;
    cur_pos_b += (pos_b - cur_pos_b) * duration * dt;

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MaterialRGBAnimation::load_config(ConfigReader &cfg)
{
    std::string secName = "MaterialRGBAnimation";

    cfg.getValue(secName, "SignalID", signal_id);
    cfg.getValue(secName, "Duration", duration);
    is_fixed_signal = cfg.getValue(secName, "FixedSignal", fixed_signal);

    std::string tmp;
    cfg.getValue(secName, "EmissionColor", tmp);

    std::istringstream ss(tmp);

    ss >> emission_color.r() >>  emission_color.g() >> emission_color.b();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float MaterialRGBAnimation::getChannelState(float pos, unsigned char channel)
{
    float channel_state = 0.0f;

    unsigned char mask = static_cast<unsigned char>(pos);

    channel_state = static_cast<float>( mask & (1 << channel) );

    return channel_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MaterialRGBAnimation::update()
{
    pos_r = getChannelState(pos, 0);
    pos_g = getChannelState(pos, 1);
    pos_b = getChannelState(pos, 2);

    osg::Vec4 new_color;
    new_color.r() = color.r() * interpolate(cur_pos_r);
    new_color.g() = color.g() * interpolate(cur_pos_g);
    new_color.b() = color.b() * interpolate(cur_pos_b);
    new_color.a() = 1.0f;

    osg::Vec4 new_emission_color;
    new_emission_color.r() = emission_color.r() * cur_pos_r;
    new_emission_color.g() = emission_color.g() * cur_pos_g;
    new_emission_color.b() = emission_color.b() * cur_pos_b;
    new_emission_color.a() = 1.0;

    mat->setDiffuse(osg::Material::FRONT_AND_BACK, new_color);
    mat->setAmbient(osg::Material::FRONT_AND_BACK, new_color);
    mat->setEmission(osg::Material::FRONT_AND_BACK, new_emission_color);

    stateset->setAttribute(mat);
    drawable->setStateSet(stateset.get());
}
