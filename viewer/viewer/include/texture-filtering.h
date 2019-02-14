#ifndef     TEXTURE_FILTERING_H
#define     TEXTURE_FILTERING_H

#include    <osg/NodeVisitor>
#include    <osg/Drawable>
#include    <osg/Geode>
#include    <osg/Texture2D>

class ModelTextureFilter : public osg::NodeVisitor
{
public:

    ModelTextureFilter() : osg::NodeVisitor()
    {
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }

    virtual void apply(osg::Geode &geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Drawable  *drawable = geode.getDrawable(i);
            osg::StateSet *stateset = drawable->getOrCreateStateSet();
            osg::StateAttribute *stateattr = stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
            osg::Texture2D *texture = static_cast<osg::Texture2D *>(stateattr);

            if (texture == nullptr)
                continue;

            texture->setNumMipmapLevels(0);
            texture->setFilter(osg::Texture::MIN_FILTER , osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER , osg::Texture::LINEAR);
        }

        traverse(geode);
    }
};

#endif // TEXTURE_FILTERING_H
