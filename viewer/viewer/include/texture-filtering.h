//------------------------------------------------------------------------------
//
//      Visitor for mipmap of textures switching off
//      (c) maisvendoo, 15/02/2019
//
//------------------------------------------------------------------------------

#ifndef     TEXTURE_FILTERING_H
#define     TEXTURE_FILTERING_H

#include    <osg/NodeVisitor>
#include    <osg/Drawable>
#include    <osg/Geode>
#include    <osg/Texture2D>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ModelTextureFilter : public osg::NodeVisitor
{
public:

    /// Constructor
    ModelTextureFilter();

    /// Visitor handler
    virtual void apply(osg::Geode &geode);
};

#endif // TEXTURE_FILTERING_H
