//------------------------------------------------------------------------------
//
//      Prepare of *.dmd model loading
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Prepare of *.dmd model loading
 * \copyright maisvendoo
 * \author maisvendoo
 */

#ifndef     MODEL_LOADER_H
#define     MODEL_LOADER_H

#include    <osg/PagedLOD>
#include    <osg/ProxyNode>
#include    <osgDB/ReadFile>

#include    <osg/Texture2D>

/*!
 * \struct
 * \brief Information about loaded *.dmd model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct model_info_t
{
    /// Model view distance
    float       view_distance;
    /// Model name
    std::string name;
    /// Path to *.dmd file
    std::string filepath;
    /// Path to texture image file
    std::string texture_path;
    /// Mipmap texture
    bool        mipmap;

    model_info_t()
        : view_distance(1000.0f)
        , name("")
        , filepath("")
        , texture_path("")
        , mipmap(false)
    {

    }
};

/*!
 * \fn
 * \brief Create paged LOD node for model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::PagedLOD *createLODNode(const model_info_t &model_info);

/*!
 * \fn
 * \brief Create texture from image file
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void createTexture(const std::string &texture_path, osg::Texture2D *texture);

#endif // MODEL_LOADER_H
