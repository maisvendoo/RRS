#ifndef     DISPLAY_CONFIG_H
#define     DISPLAY_CONFIG_H

#include    <QString>
#include    <osg/Geometry>

struct      display_config_t
{
    QString                         module_name;
    QString                         surface_name;
    osg::ref_ptr<osg::Vec2Array>    texcoord;

    display_config_t()
        : module_name("")
        , surface_name("")
    {

    }
};

#endif // DISPLAY_CONFIG_H
