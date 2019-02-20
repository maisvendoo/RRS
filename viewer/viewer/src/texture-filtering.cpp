#include    "texture-filtering.h"


ModelTextureFilter::ModelTextureFilter()
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

void ModelTextureFilter::apply(osg::Geode &geode)
{
    for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
    {
        // Get texture from geometry
        osg::Drawable  *drawable = geode.getDrawable(i);
        osg::StateSet *stateset = drawable->getOrCreateStateSet();

        osg::StateAttribute *stateattr =
                stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);

        osg::Texture2D *texture = static_cast<osg::Texture2D *>(stateattr);

        if (texture == nullptr)
            continue;

        // Switching off texture mipmapping
        texture->setNumMipmapLevels(0);
        texture->setFilter(osg::Texture::MIN_FILTER , osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER , osg::Texture::LINEAR);
    }

    traverse(geode);
}
