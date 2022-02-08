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

    ModelSmoother();

    virtual void apply(osg::Geode &geode);

};

#endif // MODELSMOOTH_H
