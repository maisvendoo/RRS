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

#include    "texture-loader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextureLoader::operator()(osg::Node *node, osg::NodeVisitor *nv)
{
    (void) nv;

    // Texture is not loaded early
    if (!_is_textured)
    {
        // Get node pointer
        osg::PagedLOD *pagedLOD = static_cast<osg::PagedLOD *>(node);

        // Node is visible
        if (pagedLOD->isCullingActive())
        {
            // Get node texture
            osg::StateSet *stateset = pagedLOD->getOrCreateStateSet();
            osg::StateAttribute *stateattr = stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
            osg::Texture2D *texture = static_cast<osg::Texture2D *>(stateattr);

            // Create texture from image
            if (texture)
            {
                createTexture(texture_path, texture);
            }

            // Mark this node as already textured
            _is_textured = true;
        }
    }

    // Go to traverse scene graph
    traverse(node, nv);
}
