#ifndef     OSGVRVIEWER_H
#define     OSGVRVIEWER_H

#include    <osgDB/ReadFile>
#include    <osgGA/TrackballManipulator>
#include    <osgViewer/Viewer>

#include    "openvrviewer.h"
#include    "openvreventhandler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class GraphicsWindowViewer : public osgViewer::Viewer
{
public:

    GraphicsWindowViewer(osg::ArgumentParser& arguments,
                         osgViewer::GraphicsWindow* graphicsWindow);

    virtual void eventTraversal();

private:

    osg::ref_ptr<osgViewer::GraphicsWindow> _graphicsWindow;
};

#endif // OSGVRVIEWER_H
