#ifndef     DISPLAY_VISITOR_H
#define     DISPLAY_VISITOR_H

#include    <osg/Transform>
#include    <osg/NodeVisitor>
#include    <QString>

#include    "display.h"

class DisplayVisitor : public osg::NodeVisitor
{
public:

    DisplayVisitor(AbstractDisplay *display, QString surfaceName);

    virtual void apply(osg::Transform &transform);

private:

    AbstractDisplay *display;
    QString         surfaceName;
};

#endif // DISPLAY_VISITOR_H
