#include    "skybox.h"

#include    <osg/Depth>
#include    <osg/TextureCubeMap>
#include    <osgUtil/CullVisitor>
#include    <osgDB/ReadFile>
#include    <osgDB/FileUtils>
#include    <osg/Geode>
#include    <osg/ShapeDrawable>
#include    <osg/TexGen>
#include    <osg/Texture2D>

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::Skybox()
{
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    setCullingActive(false);

    osg::StateSet *ss = getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 1.0f, 1.0f));
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    ss->setRenderBinDetails(5, "RenderBin");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::Skybox(const Skybox &copy, osg::CopyOp copyop)
    : osg::Transform (copy, copyop)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::setEnvironmentMap(unsigned int unit,
                               osg::Image *posX, osg::Image *negX,
                               osg::Image *posY, osg::Image *negY,
                               osg::Image *posZ, osg::Image *negZ)
{
    if (posX && posY && posZ && negX && negY && negZ)
    {
        osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, posX);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, posY);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, posZ);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, negX);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, negY);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, negZ);

        cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

        cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);

        cubemap->setResizeNonPowerOfTwoHint(false);
        getOrCreateStateSet()->setTextureAttributeAndModes(unit, cubemap.get());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Image *Skybox::loadImage(const std::string &path)
{
    std::string filePath = osgDB::findDataFile(path);

    if (filePath.empty())
    {
        OSG_WARN << "WARNING: File " + filePath << " is't found" << std::endl;
        return nullptr;
    }

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filePath);

    if (!image.valid())
    {
        OSG_WARN << "WARNING: File " + filePath << " loading fail" << std::endl;
        return nullptr;
    }

    return image.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *Skybox::loadSkyModel(const std::string &path)
{
    std::string filePath = osgDB::findDataFile(path);

    if (filePath.empty())
    {
        OSG_WARN << "Sky model file " << filePath << " is't found" << std::endl;
        return nullptr;
    }

    osg::ref_ptr<osg::Node> skymodel = osgDB::readNodeFile(filePath);

    if (!skymodel.valid())
    {
        OSG_WARN << "WARNING: Sky model file" << filePath << " loading fail" << std::endl;
        return nullptr;
    }

    return skymodel.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Skybox::computeLocalToWorldMatrix(osg::Matrix &matrix, osg::NodeVisitor *nv) const
{
    bool result = false;

    if (nv && nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor *cv = static_cast<osgUtil::CullVisitor *>(nv);
        matrix.preMult(osg::Matrix::translate(cv->getEyeLocal()));
        result = true;
    }
    else
    {
        result = osg::Transform::computeLocalToWorldMatrix(matrix, nv);
    }

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Skybox::computeWorldToLocalMatrix(osg::Matrix &matrix, osg::NodeVisitor *nv) const
{
    bool result = false;

    if (nv && nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor *cv = static_cast<osgUtil::CullVisitor *>(nv);
        matrix.postMult(osg::Matrix::translate(-cv->getEyeLocal()));
        result = true;
    }
    else
    {
        result = osg::Transform::computeWorldToLocalMatrix(matrix, nv);
    }

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::load(const std::string &routeDir, osg::Group *scene)
{
    FileSystem &fs = FileSystem::getInstance();

    std::string textureDir = fs.combinePath(routeDir, "textures");
    osg::Image *image = loadImage(fs.combinePath(textureDir, "sky_day.bmp"));

    if (image == nullptr)
    {
        return;
    }

    std::string modelDir = fs.combinePath(routeDir, "models");
    osg::ref_ptr<osg::Node> skymodel = loadSkyModel(fs.combinePath(modelDir, "sky.dmd"));

    if (skymodel == nullptr)
    {
        return;
    }

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage(image);
    texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::REPEAT);
    texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::REPEAT);
    texture->setUnRefImageDataAfterApply(true);

    skymodel->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
    this->addChild(skymodel);

    scene->addChild(this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::~Skybox()
{

}
