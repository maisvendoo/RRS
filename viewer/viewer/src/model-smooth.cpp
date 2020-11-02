#include    "model-smooth.h"


ModelSmoother::ModelSmoother()
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

void ModelSmoother::apply(osg::Geode &geode)
{
    for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
    {
        osg::Drawable  *drawable = geode.getDrawable(i);
        osg::Geometry  *geom = dynamic_cast<osg::Geometry *>(drawable);

        if (geom == nullptr)
            continue;

        osgUtil::SmoothingVisitor::smooth(*geom);
    }

    traverse(geode);
}

