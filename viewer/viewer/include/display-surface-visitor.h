#ifndef     DISPLAY_SURFACE_VISITOR_H
#define     DISPLAY_SURFACE_VISITOR_H

#include    "display-container.h"

#include    <osg/Geode>
#include    <osg/NodeVisitor>
#include    "QWidgetImage.h"
#include    <QString>

class DisplaySurfaceVisitor : public osg::NodeVisitor
{
public:

    DisplaySurfaceVisitor(display_container_t *dc, display_config_t display_config);

    virtual void apply(osg::Geode &geode);

private:

    display_container_t *dc;
    display_config_t    display_config;
};

#endif // DISPLAY_VISITOR_H
