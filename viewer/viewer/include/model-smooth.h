#ifndef     MODEL_SMOOTH_H
#define     MODEL_SMOOTH_H

#include    <osg/NodeVisitor>
#include    <osg/Drawable>
#include    <osg/Geode>
#include    <osg/Geometry>
#include    <osgUtil/SmoothingVisitor>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class  ModelSmoother : public osg::NodeVisitor
{
public:

    ModelSmoother() : osg::NodeVisitor ()
    {
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }

    virtual void apply(osg::Geode &geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Drawable  *drawable = geode.getDrawable(i);
            osg::Geometry  *geom = dynamic_cast<osg::Geometry *>(drawable);

            osgUtil::SmoothingVisitor::smooth(*geom);            
        }

        traverse(geode);
    }
};

#endif // MODELSMOOTH_H
