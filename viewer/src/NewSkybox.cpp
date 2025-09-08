#include "NewSkybox.h"

#include "CfgReader.h"
#include "Logger.h"
#include "datetime.h"
#include "filesystem.h"

#include <vsg/core/Array2D.h>
#include <vsg/core/Data.h>
#include <vsg/core/Object.h>
#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/StateGroup.h>
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

#include <cstdint>
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

    auto raster_state = vsg::RasterizationState::create();
    raster_state->cullMode = VK_CULL_MODE_FRONT_BIT;

    auto depth_state = vsg::DepthStencilState::create();
    depth_state->depthTestEnable = VK_TRUE;
    depth_state->depthWriteEnable = VK_FALSE;
    depth_state->depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    vsg::GraphicsPipelineStates pipeline_states = {
        vsg::VertexInputState::create(vertex_bindings_descriptions, vertex_attribute_descriptions),
        vsg::InputAssemblyState::create(),
        raster_state,
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



    // node2 = vsg::StateGroup::create();
    // node2->add(bind_graphics_pipeline);
    // node2->add(bind_descriptor_set);

    // auto transform = vsg::MatrixTransform::create();
    // node2->addChild(transform);

    // // Загружаем модель скайбокса
    // QString model_filename = "sky.gltf";
    // cfg.getString("Model", "Filename", model_filename);

    // std::string model_path = fs.getDataDir();
    // model_path = fs.combinePath(model_path, "models");
    // model_path = fs.combinePath(model_path, "default-objects");
    // model_path = fs.combinePath(model_path, model_filename.toStdString());

    // if (!vsg::fileExists(model_path))
    // {
    //     LOG_WARN("Fail to find skybox file: %s", model_path.c_str());
    //     return;
    // }

    // tinygltf::Model model;
    // tinygltf::TinyGLTF loader;
    // std::string err;
    // std::string warn;
    // bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path);

    // if (!ret)
    // {
    //     LOG_ERROR("Fail");
    // }

    // int vertex_count = model.accessors[0].count;

    // auto positions = vsg::vec3Array::create(vertex_count);
    // auto tex_coords = vsg::vec2Array::create(vertex_count);

    // int positions_buffer_view = model.accessors[0].bufferView;
    // int tex_coords_buffer_view = model.accessors[1].bufferView;

    // std::size_t positions_byte_offset = model.bufferViews[positions_buffer_view].byteOffset;
    // std::size_t tex_coords_byte_offset = model.bufferViews[tex_coords_buffer_view].byteOffset;

    // for (int i = 0; i < vertex_count; ++i)
    // {
    //     float x, y, z;
    //     float u, v;

    //     std::memcpy(&x, model.buffers[0].data.data() + positions_byte_offset + i * 3 * sizeof(float) + 0 * sizeof(float), sizeof(float));
    //     std::memcpy(&y, model.buffers[0].data.data() + positions_byte_offset + i * 3 * sizeof(float) + 1 * sizeof(float), sizeof(float));
    //     std::memcpy(&z, model.buffers[0].data.data() + positions_byte_offset + i * 3 * sizeof(float) + 2 * sizeof(float), sizeof(float));

    //     std::memcpy(&u, model.buffers[0].data.data() + tex_coords_byte_offset + i * 2 * sizeof(float) + 0 * sizeof(float), sizeof(float));
    //     std::memcpy(&v, model.buffers[0].data.data() + tex_coords_byte_offset + i * 2 * sizeof(float) + 1 * sizeof(float), sizeof(float));

    //     positions->at(i).set(x, y, z);
    //     tex_coords->at(i).set(u, v);
    // }

    // int index_count = model.accessors[2].count;
    // int indices_buffer_view = model.accessors[2].bufferView;
    // std::size_t indices_byte_offset = model.bufferViews[indices_buffer_view].byteOffset;

    // auto indices = vsg::ushortArray::create(index_count);
    // for (int i = 0; i < index_count; ++i)
    // {
    //     std::memcpy(&indices->at(i), model.buffers[0].data.data() + indices_byte_offset + i * sizeof(unsigned short), sizeof(unsigned short));
    // }

    // // setup geometry
    // auto drawCommands = vsg::Commands::create();

    // drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{positions, tex_coords}));
    // drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    // drawCommands->addChild(vsg::DrawIndexed::create(index_count, 1, 0, 0, 0));

    // transform->addChild(drawCommands);
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
