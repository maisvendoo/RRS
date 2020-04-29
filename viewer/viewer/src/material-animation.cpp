#include    "material-animation.h"

#include    <sstream>

MaterialAnimation::MaterialAnimation(osg::Material *mat, osg::Drawable *drawable)
    : ProcAnimation()
    , mat(new osg::Material)
    , drawable(drawable)
    , stateset(new osg::StateSet(*drawable->getOrCreateStateSet(), osg::CopyOp::DEEP_COPY_STATESETS))
    , cur_pos(0.0f)
    , color(mat->getDiffuse(osg::Material::FRONT_AND_BACK))
    , emission_color(osg::Vec4(0.0, 0.0, 0.0, 1.0))
{
    pos = 0.0f;
    duration = 0.0f;
}

MaterialAnimation::~MaterialAnimation()
{

}

void MaterialAnimation::anim_step(float t, float dt)
{
    (void) t;

    cur_pos += (pos - cur_pos) * duration * dt;

    update();
}

bool MaterialAnimation::load_config(ConfigReader &cfg)
{
    std::string secName = "MaterialAnimation";

    cfg.getValue(secName, "SignalID", signal_id);
    cfg.getValue(secName, "Duration", duration);
    is_fixed_signal = cfg.getValue(secName, "FixedSignal", fixed_signal);

    std::string tmp;
    cfg.getValue(secName, "EmissionColor", tmp);

    std::istringstream ss(tmp);

    ss >> emission_color.r() >>  emission_color.g() >> emission_color.b();

    return true;
}

void MaterialAnimation::update()
{
    osg::Vec4 new_color = color * interpolate(cur_pos);
    new_color.a() = 1.0;

    osg::Vec4 new_emission_color = emission_color * cur_pos;
    new_emission_color.a() = 1.0;

    mat->setDiffuse(osg::Material::FRONT_AND_BACK, new_color);
    mat->setAmbient(osg::Material::FRONT_AND_BACK, new_color);
    mat->setEmission(osg::Material::FRONT_AND_BACK, new_emission_color);

    stateset->setAttribute(mat);
    drawable->setStateSet(stateset.get());
}
