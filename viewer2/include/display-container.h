// #ifndef     DISPLAY_CONTAINER_H
// #define     DISPLAY_CONTAINER_H

// #include    "QWidgetImage.h"

// #include    <osgViewer/ViewerEventHandlers>
// #include    <osg/Texture2D>

// #include    "display.h"

// //------------------------------------------------------------------------------
// //
// //------------------------------------------------------------------------------
// struct  display_container_t
// {
//     AbstractDisplay                                     *display;
//     osg::ref_ptr<osgQt::QWidgetImage>                   widgetImage;
//     osg::ref_ptr<osg::Texture2D>                        texture;
//     osg::ref_ptr<osgViewer::InteractiveImageHandler>    handler;

//     display_container_t()
//         : display(nullptr)
//     {

//     }
// };

// //------------------------------------------------------------------------------
// //
// //------------------------------------------------------------------------------
// using displays_t = std::vector<display_container_t *>;

// #endif // DISPLAY_CONTAINER_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef DISPLAY_CONTAINER_H
#define DISPLAY_CONTAINER_H

#include "display.h"

#include <vector>

struct display_container_t
{
    AbstractDisplay* display;
};

using displays_t = std::vector<display_container_t*>;

#endif // DISPLAY_CONTAINER_H
