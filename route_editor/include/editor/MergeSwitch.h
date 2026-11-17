#ifndef EDITOR_MERGE_SWITCH_H
#define EDITOR_MERGE_SWITCH_H

#include <vsg/app/CompileManager.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/threading/OperationQueue.h>

#include <functional>

namespace vsg
{

class Node;
class Switch;
class Viewer;

}

struct MergeSwitch : public vsg::Inherit<vsg::Operation, MergeSwitch>
{
    MergeSwitch(vsg::observer_ptr<vsg::Viewer> viewer,
                vsg::ref_ptr<vsg::Switch> attachment_point,
                vsg::Mask mask,
                vsg::ref_ptr<vsg::Node> node,
                const vsg::CompileResult& compile_result,
                std::function<void ()> after_merge = 0);

    virtual ~MergeSwitch() override;

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Switch> attachment_point;
    vsg::Mask mask;
    vsg::ref_ptr<vsg::Node> node;
    vsg::CompileResult compile_result;
    std::function<void ()> after_merge;

    virtual void run() override;
};

#endif // EDITOR_MERGE_SWITCH_H
