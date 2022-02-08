#include    "material-rgb-animation-visitor.h"
#include    "material-rgb-animation.h"

#include    <osg/Material>

MaterialRGBAnimationVisitor::MaterialRGBAnimationVisitor(animations_t *animations,
                                                         ConfigReader *cfg)
    : osg::NodeVisitor()
    , animations(animations)
    , cfg(cfg)
{

}

void MaterialRGBAnimationVisitor::apply(osg::Geode &node)
{
    for (unsigned int j = 0; j < node.getNumDrawables(); ++j)
    {
        osg::Drawable *drawable = node.getDrawable(j);

        if (drawable == nullptr)
            continue;

        osg::StateSet *stateset = drawable->getOrCreateStateSet();
        osg::StateAttribute *stateattr = stateset->getAttribute(osg::StateAttribute::MATERIAL);

        osg::Material *mat = static_cast<osg::Material *>(stateattr);

        if (mat == nullptr)
            continue;

        ProcAnimation *animation = new MaterialRGBAnimation(mat, drawable);
        animation->load(*cfg);
        animations->insert(animation->getSignalID(), animation);
    }

    traverse(node);
}
