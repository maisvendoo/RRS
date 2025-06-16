#include "LoadModelOperation.h"

#include "CfgReader.h"
#include "animations-list.h"
#include "FindModelAnimations.h"
#include "FindCustomAnimationsVisitor.h"
#include "Logger.h"
#include "display-config.h"
#include "display-container.h"
#include "display-loader.h"
#include "filesystem.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Object.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
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
LoadModelOperation::LoadModelOperation(vsg::ref_ptr<vsg::Viewer> in_viewer,
                                       vsg::ref_ptr<vsg::Group> in_attachment_point,
                                       const std::string& in_model_filename_path,
                                       const std::string& in_animations_dir,
                                       vsg::ref_ptr<vsg::Options> in_options,
                                       vsg::ref_ptr<animations_t> in_animations) noexcept
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

    // if (!cfg_dir.empty())
    // {
    //     load_displays(node);
    // }

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

void LoadModelOperation::load_displays(vsg::ref_ptr<vsg::Node> model)
{
    FileSystem& fs = FileSystem::getInstance();

    // std::string relative_config_path = cfg_dir + fs.separator() + "displays.xml";
    std::string relative_config_path;
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);

    CfgReader cfg;
    if (cfg.load(cfg_path.c_str()))
    {
        LOG_INFO("Loaded file %s", cfg_path.c_str());
    }
    else
    {
        LOG_WARN("File %s is not found", cfg_path.c_str());
        return;
    }

    QDomNode config_section = cfg.getFirstSection("Display");
    while (!config_section.isNull())
    {
        display_config_t display_config;

        QString module_dir_name;
        cfg.getString(config_section, "ModuleDir", module_dir_name);
        std::string module_dir = fs.combinePath(fs.getModulesDir(), module_dir_name.toStdString());

        QString module_name;
        cfg.getString(config_section, "Module", module_name);
        display_config.module_name = fs.combinePath(module_dir, module_name.toStdString()).c_str();

        cfg.getString(config_section, "SurfaceName", display_config.surface_name);
        cfg.getDouble(config_section, "UpdateInterval", display_config.update_interval);

        std::vector<vsg::vec2> texcoord;
        QString corner_string;
        for (std::size_t i = 0; cfg.getString(config_section, QString("Corner%1").arg(i + 1), corner_string); ++i)
        {
            std::stringstream ss(corner_string.toStdString());
            float x, y;
            ss >> x >> y;
            texcoord.emplace_back(vsg::vec2(x, y));
        }
        display_config.texcoord = vsg::vec2Array::create(texcoord.size(), texcoord.data());

        display_container_t* dc = new display_container_t;
        loadDisplayModule(display_config, dc, model);

        config_section = cfg.getNextSection();
    }
}
