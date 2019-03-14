#ifndef     MODEL_TEXTURING_H
#define     MODEL_TEXTURING_H

#include    <osg/NodeVisitor>
#include    <osg/Texture2D>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ModelTexturing : public osg::NodeVisitor
{
public:

    ModelTexturing(const std::string &texturesDir);

    virtual void apply(osg::Geode &geode);

private:

    std::string texturesDir;

    void createTexture(const std::string &textureDir, osg::Texture2D *texture);

    void changeTexture(const std::string &textureDir, osg::Texture2D *texture);
};

#endif // MODEL_TEXTURING_H
