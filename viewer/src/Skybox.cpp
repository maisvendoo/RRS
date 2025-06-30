#include "Skybox.h"

#include <vsg/state/ShaderStage.h>
#include <vsg/io/read.h>
#include <vsgXchange/all.h>

#include "filesystem.h"
#include "Logger.h"
#include "vsg/state/GraphicsPipeline.h"
#include "vsg/state/DepthStencilState.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::Skybox(std::string& skybox_model_filepath, vsg::ref_ptr<vsg::Options> options)
{
    init(skybox_model_filepath, options);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> Skybox::getNode() const noexcept
{
    return node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::init(std::string &skybox_model_filepath, vsg::ref_ptr<vsg::Options> options)
{
    if (!vsg::fileExists(skybox_model_filepath))
    {
        LOG_WARN("Fail to find skybox file: %s", skybox_model_filepath.c_str());
        return;
    }

    vsg::ref_ptr<vsg::Object> loaded = vsg::read(skybox_model_filepath, options);
    node = loaded.cast<vsg::Node>();
    if (!node)
    {
        LOG_WARN("Fail to load skybox model from file: %s", skybox_model_filepath.c_str());

        vsg::ref_ptr<vsg::ReadError> error = loaded.cast<vsg::ReadError>();
        if (error)
            LOG_WARN(error->message.c_str());
        return;
    }

    LOG_INFO("Loaded skybox model from file: %s", skybox_model_filepath.c_str());

    FileSystem& fs = FileSystem::getInstance();
    std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    // Загружаем свой вариант вершинного шейдера вместо встроенного
    std::string shader_vert_path = shaders_dir_path + fs.separator() + "skybox_vert.vert";
    vsg::ref_ptr<vsg::ShaderStage> shader_vert_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", shader_vert_path, options);
    if (shader_vert_stage)
    {
        LOG_INFO("Loaded vertex shader for skybox: %s", shader_vert_path.c_str());
    }
    else
    {
        LOG_WARN("Fail to load vertex shader for skybox: %s", shader_vert_path.c_str());
        node = nullptr;
        return;
    }

    // Применяем настройки графического конвейера к модели неба
    struct changePipeline : public vsg::Visitor
    {
        vsg::ref_ptr<vsg::ShaderStage> stage = nullptr;
        changePipeline(vsg::ref_ptr<vsg::ShaderStage> in_stage)
            : stage(in_stage)
        {}

        void apply(vsg::Object& object)
        {
            object.traverse(*this);
        }

        void apply(vsg::BindGraphicsPipeline& bgp)
        {
            if (stage)
            {
                bgp.pipeline->stages.front() = stage;
                for (auto& state : bgp.pipeline->pipelineStates)
                {
                    if (auto dsc = state->cast<vsg::DepthStencilState>())
                    {
                        dsc->depthTestEnable = VK_FALSE;
                    }
                }
            }
        }

    } change_pipeline(shader_vert_stage);

    node->accept(change_pipeline);
}
