//------------------------------------------------------------------------------
//
//      Callback functor for visible object's texture loading
//      (c) maisvendoo, 27/11/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Callback functor for visible object's texture loading
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 27/11/2018
 */

#ifndef     TEXTURE_LOADER_H
#define     TEXTURE_LOADER_H

#include    <osg/NodeCallback>
#include    <osg/PagedLOD>
#include    <osg/Texture2D>

#include    "model-loader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TextureLoader : public osg::NodeCallback
{
public:

    /// Constructor
    TextureLoader(const std::string &texture_path)
        : texture_path(texture_path)
        , _is_textured(false)
    {

    }

    /// Callback method for texture loading
    virtual void operator() (osg::Node *node, osg::NodeVisitor *nv);

private:

    /// Texture image path
    std::string texture_path;
    /// Flag for check is object textured
    bool _is_textured;
};

#endif // TEXTURE_LOADER_H
