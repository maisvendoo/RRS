#include "Skybox.h"

#include <vsgXchange/all.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/io/read.h>
#include <vsg/core/Array2D.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include "vsg/state/GraphicsPipeline.h"
#include "vsg/state/DepthStencilState.h"

#include "filesystem.h"
#include "Logger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::Skybox(std::string& skybox_model_filepath,
               std::vector<std::string> skybox_texture_filepath,
               vsg::ref_ptr<vsg::Options> options)
{
    init_model(skybox_model_filepath, options);
    init_textures(skybox_texture_filepath, options);
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
vsg::ref_ptr<vsg::ubvec4Array2D> Skybox::getDefaultTexture() const noexcept
{
    return texture;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> Skybox::getTextures() const noexcept
{
    return textures;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::init_model(std::string &skybox_model_filepath, vsg::ref_ptr<vsg::Options> options)
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::init_textures(std::vector<std::string> skybox_texture_filepath, vsg::ref_ptr<vsg::Options> options)
{
    if (!node)
    {
        return;
    }

    // Находим дефолтную текстуру в модели
    struct findTexture : public vsg::Visitor
    {
        vsg::ref_ptr<vsg::ubvec4Array2D> texture = nullptr;
        findTexture()
        {}

        void apply(vsg::Object& object)
        {
            object.traverse(*this);
        }

        void apply(vsg::BindDescriptorSet &bindDescriptorSet)
        {
            for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
            {
                if (auto descriptor_image = descriptor.cast<vsg::DescriptorImage>())
                {
                    if ((descriptor_image->dstBinding == 0) && (descriptor_image->imageInfoList.size() > 0))
                    {
                        if (auto image_info = descriptor_image->imageInfoList[0])
                        {
                            if (auto image_view = image_info->imageView)
                            {
                                if (auto image = image_view->image)
                                {
                                    texture = image->data.cast<vsg::ubvec4Array2D>();
                                }
                            }
                        }
                    }
                }
            }
        }
    } find_texture;

    node->accept(find_texture);
    texture = find_texture.texture;

    if (!texture)
    {
        LOG_WARN("Fail to find default skybox texture in model");
        return;
    }

    // Загружаем варианты текстур
    for (auto& path : skybox_texture_filepath)
    {
        if (!vsg::fileExists(path))
        {
            LOG_WARN("Fail to find skybox texture file: %s", path.c_str());
            continue;
        }

        vsg::ref_ptr<vsg::Object> loaded = vsg::read(path, options);
        vsg::ref_ptr<vsg::ubvec4Array2D> data = loaded.cast<vsg::ubvec4Array2D>();
        if (!data)
        {
            LOG_WARN("Fail to load skybox texture from file: %s", path.c_str());

            vsg::ref_ptr<vsg::ReadError> error = loaded.cast<vsg::ReadError>();
            if (error)
                LOG_WARN(error->message.c_str());
            continue;
        }

        // Проверяем, что новая текстура совпадает по размеру с дефолтной
        if ((data->width() == texture->width()) && data->height() == texture->height())
        {
            textures.push_back(data);
        }
        else
        {
            LOG_WARN("Fail to apply skybox texture from file: %s", path.c_str());
        }
    }

    // Указываем vsg что возможна подмена текстуры скайбокса
    if (!textures.empty())
    {
        texture->properties.dataVariance = vsg::DYNAMIC_DATA;
    }
}
