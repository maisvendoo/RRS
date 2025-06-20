#include "LoadModelOperation.h"

#include "animations-list.h"
#include "FindCustomAnimationsVisitor.h"
#include "FindModelAnimations.h"
#include "Logger.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Object.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <cstddef>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MergeToScene::MergeToScene(vsg::observer_ptr<vsg::Viewer> in_viewer,
                           vsg::ref_ptr<vsg::Group> in_attachment_point,
                           vsg::ref_ptr<vsg::Node> in_node,
                           const vsg::CompileResult& in_compileResult) noexcept
    : viewer(in_viewer)
    , attachment_point(in_attachment_point)
    , node(in_node)
    , compileResult(in_compileResult)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MergeToScene::run()
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (ref_viewer && attachment_point)
    {
        // Add compiled model to viewer and scene graph
        updateViewer(*ref_viewer, compileResult);
        attachment_point->addChild(node);
    }
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LoadModelOperation::LoadModelOperation(
    vsg::ref_ptr<vsg::Viewer> in_viewer,
    vsg::ref_ptr<vsg::Group> in_attachment_point,
    const std::string& in_model_filename_path,
    const std::string& in_animations_dir,
    vsg::ref_ptr<vsg::Options> in_options,
    vsg::ref_ptr<animations_t> in_animations
) noexcept
    : viewer(in_viewer)
    , attachment_point(in_attachment_point)
    , model_filename_path(in_model_filename_path)
    , animations_dir(in_animations_dir)
    , options(in_options)
    , animations(in_animations)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LoadModelOperation::run()
{
    if (!vsg::fileExists(model_filename_path))
    {
        LOG_WARN("Operation: fail to find file: %s", model_filename_path.c_str());
        return;
    }

    auto loaded = vsg::read(model_filename_path, options);
    auto node = loaded.cast<vsg::Node>();
    if (!node)
    {
        LOG_WARN("Operation: fail to load model from file: %s", model_filename_path.c_str());

        auto error = loaded.cast<vsg::ReadError>();
        if (error)
        {
            LOG_WARN(error->message.c_str());
        }

        return;
    }

    LOG_INFO("Operation: loaded model from file: %s", model_filename_path.c_str());

    vsg::ref_ptr<animations_t> ref_animations = vsg::ref_ptr<animations_t>(animations);
    std::size_t old_size = ref_animations->animations.size();
    {
        // Model's animations
        FindModelAnimationsCreateInfo fma_create_info = {node, ref_animations, animations_dir};
        auto find_model_animations = FindModelAnimations::create(fma_create_info);
        LOG_INFO("Operation: loaded %zu (total: %zu) model animations from %s",
            ref_animations->animations.size() - old_size,
            ref_animations->animations.size(),
            animations_dir.c_str()
        );
    }

    old_size = ref_animations->animations.size();

    // Custom animations for model
    auto pdo = vsg::PropagateDynamicObjects::create();

    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    FindCustomAnimationsVisitorCreateInfo fcav_create_info = {pdo, duplicate, animations_dir, ref_animations};

    FindCustomAnimationsVisitor fcav(fcav_create_info);
    node->accept(fcav);
    LOG_INFO("Operation: loaded %zu (total: %zu) custom animations from %s",
        ref_animations->animations.size() - old_size,
        ref_animations->animations.size(),
        animations_dir.c_str()
    );

    node->traverse(*pdo);

    // Copy all animated parts of shared model for independent behaviour
    if (!pdo->dynamicObjects.empty())
    {
        for (auto& object : pdo->dynamicObjects)
        {
            if (!duplicate->contains(object))
            {
                duplicate->insert(object);
            }
        }

        duplicate->insert(node);
        node = copyop(node);
        fcav.reconfigure_animations();
    }

    // Compile loaded model and add it to viewer
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (ref_viewer && ref_viewer->compileManager)
    {
        if (auto compile_result = ref_viewer->compileManager->compile(node))
        {
            ref_viewer->addUpdateOperation(MergeToScene::create(viewer, attachment_point, node, compile_result));
        }
    }
}
