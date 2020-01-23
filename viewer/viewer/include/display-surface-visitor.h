#ifndef     DISPLAY_SURFACE_VISITOR_H
#define     DISPLAY_SURFACE_VISITOR_H

#include    "display.h"

#include    <osg/Geode>
#include    <osg/NodeVisitor>
#include    <osgQt/QWidgetImage>
#include    <QString>

class DisplaySurfaceVisitor : public osg::NodeVisitor
{
public:

    DisplaySurfaceVisitor(osgQt::QWidgetImage *widgetImage);

    virtual void apply(osg::Geode &geode);

private:

    osgQt::QWidgetImage *widgetImage;
};

#endif // DISPLAY_VISITOR_H
