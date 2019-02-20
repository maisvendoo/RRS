#include    "hud.h"
#include    "filesystem"

#include    <osg/Geode>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HUD::HUD(int width, int height, QObject *parent) : QObject (parent)
  , camera(nullptr)
  , scene(new osg::Switch)
{
    camera = createCamera(width, height);
}

osg::Camera *HUD::getCamera()
{
    return camera.get();
}

osg::Switch *HUD::getScene()
{
    return scene.get();
}

osg::Camera *HUD::createCamera(int width, int height)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    return camera.release();
}
