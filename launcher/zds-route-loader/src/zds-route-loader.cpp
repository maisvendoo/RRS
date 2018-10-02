#include    "zds-route-loader.h"

#include    <osgDB/ReadFile>
#include    <osg/Image>
#include    <osg/Texture2D>
#include    <osg/TexMat>
#include    <osg/Material>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZdsRouteLoader::ZdsRouteLoader(QObject *parent) : RouteLoader(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZdsRouteLoader::~ZdsRouteLoader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::load(QString route_path)
{
    osg::Node *routeRoot = new osg::Group;

    routeRootPath = fs->combinePath(fs->getRoutesDirectory(), route_path);

    return routeRoot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::loadModel(QString model_path, QString texture_path)
{
    osg::Node *modelNode = osgDB::readNodeFile(fs->combinePath(routeRootPath, model_path).toStdString());

    osg::StateSet   *stateset = new osg::StateSet;
    osg::Material   *material = new osg::Material;
    stateset->setAttribute(material);

    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));

    loadTexture(texture_path, stateset, 0);

    modelNode->setStateSet(stateset);

    return modelNode;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadTexture(QString texture_path,
                                 osg::StateSet *stateset,
                                 const unsigned int texture_unit)
{
    if (!texture_path.toStdString().empty())
    {
        osg::ref_ptr<osg::Image> image;
        image = osgDB::readRefImageFile(fs->combinePath(routeRootPath, texture_path).toStdString());

        if (image.valid())
        {
            osg::Texture2D *texture = new osg::Texture2D(image.get());
            osg::Texture::WrapMode textureWrapMode = osg::Texture::REPEAT;

            texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
            stateset->setTextureAttributeAndModes(texture_unit, texture,
                                                  osg::StateAttribute::ON);


            osg::Matrix mat;
            osg::TexMat *texmat = new osg::TexMat;
            texmat->setMatrix(mat);
            stateset->setTextureAttributeAndModes(texture_unit, texmat,
                                                  osg::StateAttribute::ON);
        }
    }
}

GET_ROUTE_LOADER(ZdsRouteLoader)
