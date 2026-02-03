#ifndef SINGLE_SWITCH_H
#define SINGLE_SWITCH_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

class ConstVisitor;
class RecordTraversal;
class Visitor;

}

class SingleSwitch : public vsg::Inherit<vsg::Node, SingleSwitch>
{
public:
    SingleSwitch() = default;

    SingleSwitch(vsg::Mask mask, vsg::ref_ptr<vsg::Node> node)
        : mask(mask), node(node)
    {
    }

    vsg::Mask mask = vsg::MASK_ALL;
    vsg::ref_ptr<vsg::Node> node;

    template <class N, class V>
    static void t_traverse(N& node, V& visitor)
    {
        if (node.node && (visitor.traversalMask & (
            visitor.overrideMask | node.mask)) != vsg::MASK_OFF)
        {
            node.node->accept(visitor);
        }
    }

    void traverse(vsg::Visitor& visitor) override
    {
        t_traverse(*this, visitor);
    }

    void traverse(vsg::ConstVisitor& visitor) const override
    {
        t_traverse(*this, visitor);
    }

    void traverse(vsg::RecordTraversal& visitor) const override
    {
        t_traverse(*this, visitor);
    }
};

#endif // SINGLE_SWITCH_H
