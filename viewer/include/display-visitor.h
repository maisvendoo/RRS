// #ifndef     DISPLAY_VISITOR_H
// #define     DISPLAY_VISITOR_H

// #include    <osg/Transform>
// #include    <osg/NodeVisitor>
// #include    <QString>

// #include    "display-container.h"

// #include    "display-config.h"

// class DisplayVisitor : public osg::NodeVisitor
// {
// public:

//     DisplayVisitor(display_container_t *dc, display_config_t display_config);

//     virtual void apply(osg::Transform &transform);

// private:

//     display_container_t *dc;
//     display_config_t    display_config;
// };

// #endif // DISPLAY_VISITOR_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef DISPLAY_VISITOR_H
#define DISPLAY_VISITOR_H

#include "display-config.h"
#include "display-container.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>

class DisplayVisitor final : public vsg::Inherit<vsg::Visitor, DisplayVisitor>
{
public:
    DisplayVisitor(display_container_t* dc, const display_config_t& display_config);

    void apply(vsg::Node& transform) override;

private:
    display_container_t* dc;
    display_config_t display_config;
};

#endif // DISPLAY_VISITOR_H
