#include    "material-animation-visitor.h"
#include    "material-animation.h"

#include    <osg/Material>

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t *animations, ConfigReader *cfg)
    : osg::NodeVisitor()
    , animations(animations)
    , cfg(cfg)
{

}

void MaterialAnimationVisitor::apply(osg::Geode &geode)
{
    for (unsigned int j = 0; j < geode.getNumDrawables(); ++j)
    {
        osg::Drawable *drawable = geode.getDrawable(j);

        if (drawable == nullptr)
            continue;

        osg::StateSet *stateset = drawable->getOrCreateStateSet();
        osg::StateAttribute *stateattr = stateset->getAttribute(osg::StateAttribute::MATERIAL);

        osg::Material *mat = static_cast<osg::Material *>(stateattr);

        if (mat == nullptr)
            continue;

        ProcAnimation *animation = new MaterialAnimation(mat, drawable);
        animation->load(*cfg);
        animations->insert(animation->getSignalID(), animation);
    }

    traverse(geode);
}
