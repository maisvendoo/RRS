// #ifndef     DISPLAY_CONFIG_H
// #define     DISPLAY_CONFIG_H

// #include    <QString>
// #include    <osg/Geometry>

// struct      display_config_t
// {
//     QString                         module_name;
//     QString                         surface_name;
//     double                          update_interval;
//     osg::ref_ptr<osg::Vec2Array>    texcoord;

//     display_config_t()
//         : module_name("")
//         , surface_name("")
//         , update_interval(0.5)
//     {

//     }
// };

// #endif // DISPLAY_CONFIG_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <vsg/core/Array.h>
#include <vsg/core/ref_ptr.h>

#include <QString>

struct display_config_t
{
    QString module_name = "";
    QString surface_name = "";
    double update_interval = 0.5;
    vsg::ref_ptr<vsg::vec2Array> texcoord;
};

#endif // DISPLAY_CONFIG_H
