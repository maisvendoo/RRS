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
    sun->setAmbient(color *= power * 0.0f);
    sun->setSpecular(color *= power);


    float dist = 1000.0f;

    float rad = osg::PIf / 180.0f;
    float x = dist * cosf(theta * rad) * sinf(psi * rad);
    float y = dist * cosf(theta * rad) * cosf(psi * rad);
    float z = dist * sinf(theta * rad);

    osg::Vec3 pos = osg::Vec3(x, y, z);
    sun->setPosition(osg::Vec4(pos, 0.0f));

    osg::Vec3 sunDir = pos *= (- 1.0f / pos.length());
    sun->setDirection(sunDir);

    osg::ref_ptr<osg::LightSource> light0 = new osg::LightSource;
    light0->setLight(sun);    

    root->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);
    root->addChild(light0.get());
}
