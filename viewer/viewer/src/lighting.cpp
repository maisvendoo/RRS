//------------------------------------------------------------------------------
//
//      Global environment lighting (Sun emulation)
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Global environment lighting (Sun emulation)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#include	"lighting.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void initEnvironmentLight(osg::Group *root,
                          osg::Vec4 color,
                          float power,
                          float psi,
                          float theta)
{
    osg::ref_ptr<osg::Light> sun = new osg::Light;
    sun->setLightNum(0);
    sun->setDiffuse(color *= power);
    sun->setAmbient(color *= power);
    sun->setSpecular(color *= power);
    sun->setPosition(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));

    float rad = osg::PIf / 180.0f;
    float dx = -cosf(theta * rad) * sinf(psi * rad);
    float dy = -cosf(theta * rad) * cosf(psi * rad);
    float dz = -sinf(theta * rad);
    osg::Vec3 sunDir(dx, dy, dz);
    sun->setDirection(sunDir);

    osg::ref_ptr<osg::LightSource> light0 = new osg::LightSource;
    light0->setLight(sun);

    root->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);
    root->addChild(light0.get());
}
