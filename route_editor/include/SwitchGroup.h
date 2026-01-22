#ifndef SWITCH_GROUP_H
#define SWITCH_GROUP_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Mask.h>
#include <vsg/nodes/Switch.h>

class SwitchGroup : public vsg::Inherit<vsg::Switch, SwitchGroup>
{
public:
    void set_mask(vsg::Mask mask);
};

#endif // SWITCH_GROUP_H
