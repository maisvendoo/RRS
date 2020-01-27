#ifndef     DISPLAY_VISITOR_H
#define     DISPLAY_VISITOR_H

#include    <osg/Transform>
#include    <osg/NodeVisitor>
#include    <QString>

#include    "display-container.h"

class DisplayVisitor : public osg::NodeVisitor
{
public:

    DisplayVisitor(display_container_t *dc, display_config_t display_config);

    virtual void apply(osg::Transform &transform);

private:

    display_container_t *dc;
    display_config_t    display_config;
};

#endif // DISPLAY_VISITOR_H
