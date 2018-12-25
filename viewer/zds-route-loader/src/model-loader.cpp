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

#include    "model-loader.h"

#include    <osg/Texture2D>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ImageOptions>

#include    "texture-loader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void createTexture(const std::string &texture_path, osg::Texture2D *texture)
{
    // Check file path
    if (texture_path.empty())
        return;

    std::string fileName = osgDB::findDataFile(texture_path,
                                               osgDB::CaseSensitivity::CASE_INSENSITIVE);

    if (fileName.empty())
    {
        OSG_WARN << "NOT FOUND: " << texture_path << std::endl;
        return;
    }

    // Load image from file
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(fileName);

    if (!image.valid())
        return;

    // Vertical flipping of *.bmp image
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);

    if (ext == "tga")
    {
        //image->flipHorizontal();
        image->flipVertical();
    }

    // Apply image for texture
    texture->setImage(image.get());    
    texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::REPEAT);
    texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::REPEAT);    
    texture->setUnRefImageDataAfterApply(true);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::PagedLOD *createLODNode(const model_info_t &model_info)
{
    // Check file path
    std::string fileName = osgDB::findDataFile(model_info.filepath,
                                               osgDB::CaseSensitivity::CASE_INSENSITIVE);

    if (fileName.empty())
    {
        OSG_WARN << "NOT FOUND: " << model_info.filepath << std::endl;
        return nullptr;
    }

    // Prepare model loading
    osg::ref_ptr<osg::PagedLOD> pagedLOD = new osg::PagedLOD;
    pagedLOD->addDescription(model_info.name);
    pagedLOD->setFileName(0, fileName);
    pagedLOD->setRange(0, 0.0f, model_info.view_distance);

    // Apply texture
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;

    pagedLOD->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());

    // Apply transparency settings
    std::string ext = osgDB::getLowerCaseFileExtension(model_info.texture_path);
    if (ext == "tga")
        pagedLOD->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    else
        pagedLOD->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);

    // Set callback for texture loading
    pagedLOD->setCullCallback(new TextureLoader(model_info.texture_path));

    return pagedLOD.release();
}
