#ifndef LOAD_MODEL_OPERATION_H
#define LOAD_MODEL_OPERATION_H

#include <vsg/app/Viewer.h>
#include <vsg/io/read.h>
// #include <vsg/nodes/DepthSorted.h>
#include <vsgXchange/all.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include "AnimTransformVisitor.h"
#include "Logger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct MergeToScene : public vsg::Inherit<vsg::Operation, MergeToScene>
{
    MergeToScene(vsg::observer_ptr<vsg::Viewer> in_viewer,
                 vsg::ref_ptr<vsg::Group> in_attachment_point,
                 vsg::ref_ptr<vsg::Node> in_node,
                 const vsg::CompileResult& in_compileResult)
        : viewer(in_viewer)
        , attachment_point(in_attachment_point)
        , node(in_node)
        , compileResult(in_compileResult)
    {

    }

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Group> attachment_point;
    vsg::ref_ptr<vsg::Node> node;
    vsg::CompileResult compileResult;

    void run() override
    {
        vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
        if (ref_viewer && attachment_point)
        {
            // Add compiled model to viewer and scene graph
            updateViewer(*ref_viewer, compileResult);

            // auto depthSorted = vsg::DepthSorted::create();
            // depthSorted->child = node;

            // attachment_point->addChild(depthSorted);
            attachment_point->addChild(node);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct LoadModelOperation : public vsg::Inherit<vsg::Operation, LoadModelOperation>
{
    LoadModelOperation(vsg::ref_ptr<vsg::Viewer> in_viewer,
                       vsg::ref_ptr<vsg::Group> in_attachment_point,
                       const std::string& in_model_filename_path,
                       const std::string& in_animations_dir,
                       const std::string& in_textures_dir, // TODO
                       vsg::ref_ptr<vsg::Options> in_options,
                       animations_t* in_animations)
        : viewer(in_viewer)
        , attachment_point(in_attachment_point)
        , model_filename_path(in_model_filename_path)
        , animations_dir(in_animations_dir)
        , textures_dir(in_textures_dir) // TODO
        , options(in_options)
        , animations(in_animations)
    {

    }

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Group> attachment_point = nullptr;
    std::string model_filename_path = "";
    std::string animations_dir = "";
    std::string textures_dir = ""; // TODO
    vsg::ref_ptr<vsg::Options> options = nullptr;
    animations_t* animations;

    void run() override
    {
        if (!vsg::fileExists(model_filename_path))
        {
            LOG_WARN("Operation: fail to find file: %s", model_filename_path.c_str());
            return;
        }

        vsg::ref_ptr<vsg::Object> loaded = vsg::read(model_filename_path, options);
        vsg::ref_ptr<vsg::Node> node = loaded.cast<vsg::Node>();
        if (!node)
        {
            LOG_WARN("Operation: fail to load model from file: %s", model_filename_path.c_str());

            vsg::ref_ptr<vsg::ReadError> error = loaded.cast<vsg::ReadError>();
            if (error)
                LOG_WARN(error->message.c_str());
            return;
        }

        LOG_INFO("Operation: loaded model from file: %s", model_filename_path.c_str());

        // Custom animations for model
        vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo = vsg::PropagateDynamicObjects::create();

        vsg::CopyOp copyop;
        copyop.duplicate = new vsg::Duplicate;
        vsg::ref_ptr<vsg::Duplicate> duplicate = copyop.duplicate;

        int old_size = animations->animations.size();

        AnimTransformVisitorCreateInfo atv_create_info = {
            .pdo = pdo,
            .duplicate = duplicate,
            .animations_dir = animations_dir,
            .animations = animations
        };

        AnimTransformVisitor atv(atv_create_info);
        node->accept(atv);
        LOG_INFO("Operation: loaded %u (total: %u) custom animations from %s",
                 animations->animations.size() - old_size,
                 animations->animations.size(),
                 animations_dir.c_str());

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
        }

        // Compile loaded model and add it to viewer
        vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
        if (ref_viewer && ref_viewer->compileManager)
        {
            if (auto compile_result = ref_viewer->compileManager->compile(node))
            {
                ref_viewer->addUpdateOperation(MergeToScene::create(viewer,
                                                                    attachment_point,
                                                                    node,
                                                                    compile_result));
            }
        }
    }
};

#endif // LOAD_MODEL_OPERATION_H
