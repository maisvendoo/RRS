#ifndef     DISPLAY_CONTAINER_H
#define     DISPLAY_CONTAINER_H

#include    <osgQt/QWidgetImage>
#include    <osgViewer/ViewerEventHandlers>
#include    <osg/Texture2D>

#include    "display.h"
#include    "display-config.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct  display_container_t
{
    AbstractDisplay                                     *display;
    osg::ref_ptr<osgQt::QWidgetImage>                   widgetImage;
    osg::ref_ptr<osg::Texture2D>                        texture;
    osg::ref_ptr<osgViewer::InteractiveImageHandler>    handler;

    display_container_t()
        : display(nullptr)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<display_container_t *> displays_t;

#endif // DISPLAY_CONTAINER_H
