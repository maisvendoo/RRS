#include "SwitchGroup.h"

#include <vsg/core/Mask.h>

void SwitchGroup::set_mask(vsg::Mask mask)
{
    for (auto& child : children)
    {
        child.mask = mask;
    }
}
