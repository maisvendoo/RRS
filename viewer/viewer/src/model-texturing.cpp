#include    "model-texturing.h"
#include    "filesystem.h"

#include    <osg/Geode>
#include    <osgDB/ReadFile>
#include    <QFileInfo>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelTexturing::ModelTexturing(const std::string &texturesDir)
    : texturesDir(texturesDir)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelTexturing::apply(osg::Geode &geode)
{
    previousPath = "";
    for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
    {
        osg::Drawable  *drawable = geode.getDrawable(i);
        osg::StateSet *stateset = drawable->getOrCreateStateSet();

        osg::StateAttribute *stateattr =
                stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);

        osg::Texture2D *texture = static_cast<osg::Texture2D *>(stateattr);

        if (texture == nullptr)
            createTexture(texturesDir, texture);
        else
            changeTexture(texturesDir, texture);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelTexturing::createTexture(const std::string &textureDir,
                                   osg::Texture2D *texture)
{
    Q_UNUSED(textureDir)
    Q_UNUSED(texture)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelTexturing::changeTexture(const std::string &textureDir,
                                   osg::Texture2D *texture)
{
    FileSystem &fs = FileSystem::getInstance();

    osg::Image *image = texture->getImage();
    std::string fullPath = fs.toNativeSeparators(image->getFileName());

    QFileInfo info(QFile(QString(fullPath.c_str())));
    std::string fileName  = info.fileName().toStdString();


    std::string newDir = fs.combinePath(fs.getVehicleModelsDir(), textureDir);
    std::string newPath = fs.combinePath(newDir, fileName);

    // Проверяем, что новая текстура не используется в модели изначально
    if (newPath == fullPath)
    {
        return;
    }

    // Проверяем, что новая текстура не использовалась в предыдущем цикле
    if (newPath != previousPath)
    {
        newImage = osgDB::readImageFile(newPath);
        previousPath = newPath;
    }

    if (newImage.valid())
        texture->setImage(newImage.get());
}


