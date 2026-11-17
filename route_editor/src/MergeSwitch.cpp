#include "editor/MergeSwitch.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

MergeSwitch::MergeSwitch(vsg::observer_ptr<vsg::Viewer> viewer,
                         vsg::ref_ptr<vsg::Switch> attachment_point,
                         vsg::Mask mask,
                         vsg::ref_ptr<vsg::Node> node,
                         const vsg::CompileResult& compile_result,
                        std::function<void ()> after_merge)
    : viewer{viewer}
    , attachment_point{attachment_point}
    , mask{mask}
    , node{node}
    , compile_result{compile_result}
    , after_merge(after_merge)
{
}

MergeSwitch::~MergeSwitch() = default;

void MergeSwitch::run()
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (ref_viewer)
    {
        vsg::updateViewer(*ref_viewer, compile_result);
    }

    attachment_point->addChild(mask, node);

    if (after_merge)
    {
        after_merge();
    }
}
