#ifndef     DISPLAY_CONTAINER_H
#define     DISPLAY_CONTAINER_H

#ifdef __WIN32__
#include    <osgQt/QWidgetImage>
#else
#include    <osgQOpenGL/osgQOpenGLWidget>
#endif

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

#ifdef __WIN32__
    osg::ref_ptr<osgQt::QWidgetImage>                   widgetImage;
#else
    osg::ref_ptr<osgQOpenGLWidget>                      widgetImage;
#endif
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
