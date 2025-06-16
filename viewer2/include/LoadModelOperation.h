#ifndef LOAD_MODEL_OPERATION_H
#define LOAD_MODEL_OPERATION_H

#include "animations-list.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>
#include <vsg/threading/OperationQueue.h>

#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct MergeToScene : public vsg::Inherit<vsg::Operation, MergeToScene>
{
    MergeToScene(vsg::observer_ptr<vsg::Viewer> in_viewer,
                 vsg::ref_ptr<vsg::Group> in_attachment_point,
                 vsg::ref_ptr<vsg::Node> in_node,
                 const vsg::CompileResult& in_compileResult) noexcept;

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Group> attachment_point;
    vsg::ref_ptr<vsg::Node> node;
    vsg::CompileResult compileResult;

    void run() override;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct LoadModelOperation : public vsg::Inherit<vsg::Operation, LoadModelOperation>
{
    LoadModelOperation(
        vsg::ref_ptr<vsg::Viewer> in_viewer,
        vsg::ref_ptr<vsg::Group> in_attachment_point,
        const std::string& in_model_filename_path,
        const std::string& in_animations_dir,
        vsg::ref_ptr<vsg::Options> in_options,
        vsg::ref_ptr<animations_t> in_animations,
        const std::string& cfg_dir = ""
    ) noexcept;

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Group> attachment_point = nullptr;
    std::string model_filename_path = "";
    std::string animations_dir = "";
    vsg::ref_ptr<vsg::Options> options = nullptr;
    vsg::observer_ptr<animations_t> animations;
    std::string cfg_dir;

    void run() override;

    void load_displays(vsg::ref_ptr<vsg::Node> model);
};

#endif // LOAD_MODEL_OPERATION_H
