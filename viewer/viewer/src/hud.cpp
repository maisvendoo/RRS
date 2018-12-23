#include    "hud.h"

HUD::HUD(QObject *parent) : QObject(parent)
{

}

HUD::~HUD()
{

}

void HUD::init(double left, double right, double bottom, double top)
{
    hud_camera = new osg::Camera;
    hud_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    hud_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    hud_camera->setRenderOrder(osg::Camera::POST_RENDER);
    hud_camera->setAllowEventFocus(false);
    hud_camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
}
