// #ifndef     DISPLAY_SURFACE_VISITOR_H
// #define     DISPLAY_SURFACE_VISITOR_H

// #include    "display-container.h"

// #include    <osg/Geode>
// #include    <osg/NodeVisitor>
// #include    <QString>

// #include    "display-config.h"

// class DisplaySurfaceVisitor : public osg::NodeVisitor
// {
// public:

//     DisplaySurfaceVisitor(display_container_t *dc, display_config_t display_config);

//     virtual void apply(osg::Geode &geode);

// private:

//     display_container_t *dc;
//     display_config_t    display_config;
// };

// #endif // DISPLAY_VISITOR_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef DISPLAY_SURFACE_VISITOR_H
#define DISPLAY_SURFACE_VISITOR_H

#include "display-config.h"
#include "display-container.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/StateGroup.h>

class DisplaySurfaceVisitor final : public vsg::Inherit<vsg::Visitor, DisplaySurfaceVisitor>
{
public:
    DisplaySurfaceVisitor(display_container_t* dc, const display_config_t& display_config);

    void apply(vsg::Node& node) override;
    void apply(vsg::StateGroup& stateGroup) override;

private:
    display_container_t* dc;
    display_config_t display_config;
};


#endif // DISPLAY_SURFACE_VISITOR_H
