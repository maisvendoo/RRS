#include "NewSkybox.h"

#include "CfgReader.h"
#include "Logger.h"
#include "datetime.h"
#include "filesystem.h"

#include <vsg/app/RecordTraversal.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Data.h>
#include <vsg/core/Object.h>
#include <vsg/core/Value.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>

#include <QDomNode>
#include <QString>
#include <QStringList>

#include <string>
#include <vector>

NewSkybox::NewSkybox(const std::string& skybox_config_filepath, vsg::ref_ptr<vsg::Options> options)
{
    CfgReader cfg;
    if (cfg.load(skybox_config_filepath.c_str()))
    {
        init_textures(cfg, options);
        if (textures.empty())
        {
            LOG_WARN("Failed to init skybox textures. Skybox config: %s", skybox_config_filepath.c_str());
            return;
        }

        init_model(cfg, options);
    }
    else
    {
        LOG_WARN("Failed to open skybox config: %s", skybox_config_filepath.c_str());
    }
}

vsg::ref_ptr<vsg::StateGroup> NewSkybox::get_state_group() const
{
    return state_group;
}

void NewSkybox::set_date_time(const simulator_time_t& sim_time)
{
    // uniform_value->set(uniform_value->value() + 0.001);
    // static bool changed = false;
    // if (uniform_value->value() > 0.5 && !changed)
    // {
    //     changed = true;

    //     memcpy(t1->dataPointer(), t2->dataPointer(), t2->dataSize());

    //     vsg::ref_ptr<vsg::Data> temp = vsg::read_cast<vsg::Data>("sky_sunrise.bmp", options);
    //     memcpy(t2->dataPointer(), temp->dataPointer(), temp->dataSize());

    //     t2->properties.dataVariance = vsg::DYNAMIC_DATA;

    //     t1->dirty();
    //     t2->dirty();
    // }
    // std::cout << uniform_value->value() << std::endl;
    // uniform_value->dirty();
}

class FindArraysVisitor : public vsg::Visitor
{
public:
    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::VertexIndexDraw& vid) override
    {
        for (auto buffer_info : vid.arrays)
        {
            auto data = buffer_info->data;
            if (auto array = data.cast<vsg::vec3Array>())
            {
                positions = array;
            }
            else if (auto array = data.cast<vsg::vec2Array>())
            {
                tex_coords = array;
            }
        }

        indices = vid.indices->data->cast<vsg::uintArray>();
    }

public:
    vsg::ref_ptr<vsg::vec3Array> positions;
    vsg::ref_ptr<vsg::vec2Array> tex_coords;
    vsg::ref_ptr<vsg::uintArray> indices;
};

void NewSkybox::init_model(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options)
{
    // Получаем пути к шейдерам скайбокса
    FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";
    const std::string shader_vert_path = shaders_dir_path + fs.separator() + "new_skybox.vert";
    const std::string shader_frag_path = shaders_dir_path + fs.separator() + "new_skybox.frag";

    // Читаем шейдеры из соответствующих путей
    auto vertex_shader = vsg::read_cast<vsg::ShaderStage>(shader_vert_path, options);
    auto fragment_shader = vsg::read_cast<vsg::ShaderStage>(shader_frag_path, options);

    if (!vertex_shader || !fragment_shader)
    {
        LOG_ERROR("Could not create shaders");
        return;
    }

    // Настраиваем пайплайн скайбокса
    vsg::DescriptorSetLayoutBindings descriptor_bindings = {
        // {binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers}
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    };

    auto descriptor_set_layout = vsg::DescriptorSetLayout::create(descriptor_bindings);

    // projection, view, and model matrices, actual push constant calls automatically provided by the VSG's RecordTraversal
    vsg::PushConstantRanges push_constant_ranges = {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128}
    };

    vsg::VertexInputState::Bindings vertex_bindings_descriptions = {
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{1, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    vsg::VertexInputState::Attributes vertex_attribute_descriptions = {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32_SFLOAT, 0}
    };

    auto depth_state = vsg::DepthStencilState::create();
    depth_state->depthTestEnable = VK_TRUE;
    depth_state->depthWriteEnable = VK_FALSE;
    depth_state->depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    vsg::GraphicsPipelineStates pipeline_states = {
        vsg::VertexInputState::create(vertex_bindings_descriptions, vertex_attribute_descriptions),
        vsg::InputAssemblyState::create(),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        depth_state
    };

    auto pipeline_layout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptor_set_layout}, push_constant_ranges);
    auto pipeline = vsg::GraphicsPipeline::create(pipeline_layout, vsg::ShaderStages{vertex_shader, fragment_shader}, pipeline_states);
    auto bind_graphics_pipeline = vsg::BindGraphicsPipeline::create(pipeline);

    texture1_data = vsg::clone(textures[0].texture);
    texture2_data = vsg::clone(textures[0].texture);
    texture1_data->properties.dataVariance = vsg::DYNAMIC_DATA;
    texture2_data->properties.dataVariance = vsg::DYNAMIC_DATA;

    auto sampler = vsg::Sampler::create();
    auto texture_descriptor1 = vsg::DescriptorImage::create(sampler, texture1_data, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    auto texture_descriptor2 = vsg::DescriptorImage::create(sampler, texture2_data, 1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    mix_value = vsg::floatValue::create(0.0f);
    mix_value->properties.dataVariance = vsg::DYNAMIC_DATA;

    auto mix_value_descriptor = vsg::DescriptorBuffer::create(mix_value, 2, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    auto descriptor_set = vsg::DescriptorSet::create(descriptor_set_layout, vsg::Descriptors{
        texture_descriptor1, texture_descriptor2, mix_value_descriptor
    });

    auto bind_descriptor_set = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_set);

    state_group = vsg::StateGroup::create();
    state_group->add(bind_graphics_pipeline);
    state_group->add(bind_descriptor_set);

    // Загружаем модель скайбокса
    QString model_filename = "sky.gltf";
    cfg.getString("Model", "Filename", model_filename);

    std::string model_path = fs.getDataDir();
    model_path = fs.combinePath(model_path, "models");
    model_path = fs.combinePath(model_path, "default-objects");
    model_path = fs.combinePath(model_path, model_filename.toStdString());

    if (!vsg::fileExists(model_path))
    {
        LOG_WARN("Failed to find skybox file: %s", model_path.c_str());
        return;
    }

    auto loaded = vsg::read(model_path, options);
    auto node = loaded.cast<vsg::Node>();
    if (!node)
    {
        LOG_WARN("Failed to load skybox model from file: %s", model_path.c_str());

        auto error = loaded.cast<vsg::ReadError>();
        if (error)
        {
            LOG_WARN(error->message.c_str());
        }

        return;
    }

    FindArraysVisitor fav;
    node->accept(fav);

    auto draw_commands = vsg::Commands::create();
    draw_commands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{fav.positions, fav.tex_coords}));
    draw_commands->addChild(vsg::BindIndexBuffer::create(fav.indices));
    draw_commands->addChild(vsg::DrawIndexed::create(fav.indices->size(), 1, 0, 0, 0));

    auto transform = vsg::MatrixTransform::create();
    transform->matrix = vsg::rotate(vsg::radians(90.0), vsg::dvec3{1.0, 0.0, 0.0});

    transform->addChild(draw_commands);
    state_group->addChild(transform);
}

void NewSkybox::init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string textures_dir_path = fs.getDataDir();
    textures_dir_path = fs.combinePath(textures_dir_path, "models");
    textures_dir_path = fs.combinePath(textures_dir_path, "default-objects");
    textures_dir_path = fs.combinePath(textures_dir_path, "textures");

    // Читаем из конфига имена файлов текстур и их сезон, время суток
    QDomNode sec_node = cfg.getFirstSection("Texture");
    while (!sec_node.isNull())
    {
        QString texture_filename = "sky_day.bmp";
        cfg.getString(sec_node, "Filename", texture_filename);
        const std::string texture_path = fs.combinePath(textures_dir_path, texture_filename.toStdString());

        // Ищем файл текстуры
        if (!vsg::fileExists(texture_path))
        {
            LOG_WARN("Failed to find skybox texture file: %s", texture_path.c_str());

            sec_node = cfg.getNextSection();
            continue;
        }

        // Загружаем файл текстуры
        auto loaded = vsg::read(texture_path, options);
        auto data = loaded->cast<vsg::ubvec4Array2D>();
        if (!data)
        {
            LOG_WARN("Failed to load skybox texture from file: %s", texture_path.c_str());

            auto error = loaded->cast<vsg::ReadError>();
            if (error)
            {
                LOG_WARN(error->message.c_str());
            }

            sec_node = cfg.getNextSection();
            continue;
        }

        // Проверяем, что новая текстура совпадает по размеру с остальными
        if (!textures.empty())
        {
            auto texture = textures[0].texture;
            if ((data->width() != texture->width()) || (data->height() != texture->height()))
            {
                LOG_WARN("Failed to apply skybox texture from file: %s", texture_path.c_str());

                sec_node = cfg.getNextSection();
                continue;
            }
        }

        LOG_INFO("Loaded skybox texture from file: %s", texture_path.c_str());

        season_time_texture_t stt{};
        stt.texture = data;
        stt.filename = texture_filename.toStdString();

        auto read_season_date = [](CfgReader& cfg, QDomNode& sec_node, const char* section,
                                   season_time_texture_t::season_date_t& sd) -> bool
        {
            QString tmp;
            if (!cfg.getString(sec_node, section, tmp))
            {
                return false;
            }

            QStringList tokens = tmp.split("-");
            if (tokens.size() != 2)
            {
                return false;
            }

            sd.month = tokens[0].toInt();
            if ((sd.month < 1) || (sd.month > 12))
            {
                return false;
            }

            sd.day = tokens[1].toInt();
            if ((sd.day < 1) || (sd.day > 31))
            {
                return false;
            }

            return true;
        };

        auto read_time = [](CfgReader& cfg, QDomNode& sec_node, const char* section, server_time_t& st) -> bool
        {
            QString tmp;
            if (!cfg.getString(sec_node, section, tmp))
            {
                return false;
            }

            QStringList tokens = tmp.split(":");
            if (tokens.size() != 3)
            {
                return false;
            }

            st = server_time_t(tokens[0].toInt(), tokens[1].toInt(), tokens[2].toInt());
            return true;
        };

        if (!read_season_date(cfg, sec_node, "SeasonBegin", stt.date_season_begin)
            || !read_season_date(cfg, sec_node, "SeasonEnd", stt.date_season_end)
            || !read_time(cfg, sec_node, "TimeAppearBegin", stt.time_appear_begin)
            || !read_time(cfg, sec_node, "TimeAppearEnd", stt.time_appear_end)
            || !read_time(cfg, sec_node, "TimeDisappearBegin", stt.time_disappear_begin)
            || !read_time(cfg, sec_node, "TimeDisappearEnd", stt.time_disappear_end))
        {
            LOG_WARN("Failed to read season or time from config for skybox texture: %s", texture_path.c_str());

            sec_node = cfg.getNextSection();
            continue;
        }

        int through_midnight_count = (stt.time_appear_begin > stt.time_appear_end)
                                     + (stt.time_appear_end > stt.time_disappear_begin)
                                     + (stt.time_disappear_begin > stt.time_disappear_end);
        if (through_midnight_count > 1)
        {
            LOG_WARN("Invalid time for skybox texture: %s", texture_path.c_str());

            sec_node = cfg.getNextSection();
            continue;
        }

        textures.emplace_back(stt);
        sec_node = cfg.getNextSection();
    }
}
