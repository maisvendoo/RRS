#ifndef     CAMERAVIEW_H
#define     CAMERAVIEW_H

#include    <osg/Camera>
#include    <osgViewer/View>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct camera_view_t
{
    osg::ref_ptr<osgViewer::View>   view;
    osg::ref_ptr<osg::Camera>       camera;
    int     x;
    int     y;
    int     width;
    int     height;
    bool    fullscreen;
    bool    show_window_title;
    int     screen_num;

    camera_view_t()
        : camera(nullptr)
        , x(0)
        , y(0)
        , width(1024)
        , height(768)
        , fullscreen(false)
        , show_window_title(true)
        , screen_num(0)
    {

    }
};

#endif // CAMERAVIEW_H
