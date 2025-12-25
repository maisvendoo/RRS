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

#include <cstddef>
#include <cstring>
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

vsg::ref_ptr<vsg::Node> NewSkybox::getNode() const
{
    return state_group;
}

void NewSkybox::set_date_time(const simulator_time_t& sim_time)
{
    is_sun_rise = sim_time.time.hour() < 13;
    return;/*
    for (season_time_texture_t& stt : textures)
    {
        // Проверяем, что дата попадет в сезон применения текстуры
        const server_date_t date_begin = {sim_time.date.year(), stt.date_season_begin.month, stt.date_season_begin.day};
        const bool is_after_season_begin = (sim_time.date >= date_begin);

        const server_date_t date_end = {sim_time.date.year(), stt.date_season_end.month, stt.date_season_end.day};
        const bool is_before_season_end = (sim_time.date <= date_end);

        const bool is_season = (date_begin > date_end)
            ? (is_after_season_begin || is_before_season_end)
            : (is_after_season_begin && is_before_season_end);

        if (!is_season)
        {
            stt.state = season_time_texture_t::State::INACTIVE;
            continue;
        }

        // Проверяем, что время попадет в интервал применения текстуры
        auto time_in_interval = [](const server_time_t& time,
                                   const server_time_t& begin,
                                   const server_time_t& end) -> bool
        {
            if (begin > end)
            {
                return (time >= begin) || (time < end);
            }
            else
            {
                return (time >= begin) && (time < end);
            }
        };

        if (time_in_interval(sim_time.time, stt.time_appear_begin, stt.time_appear_end))
        {
            if (stt.state != season_time_texture_t::State::APPEARING)
            {
                std::memcpy(texture2_data->dataPointer(), stt.texture->dataPointer(), stt.texture->dataSize());
                texture2_data->dirty();

                stt.state = season_time_texture_t::State::APPEARING;
            }
        }
        else if (time_in_interval(sim_time.time, stt.time_appear_end, stt.time_disappear_begin))
        {
            if (stt.state != season_time_texture_t::State::ACTIVE)
            {
                std::memcpy(texture2_data->dataPointer(), stt.texture->dataPointer(), stt.texture->dataSize());
                texture2_data->dirty();

                mix_value->set(1.0f);
                mix_value->dirty();

                stt.state = season_time_texture_t::State::ACTIVE;
            }
        }
        else if (time_in_interval(sim_time.time, stt.time_disappear_begin, stt.time_disappear_end))
        {
            if (stt.state != season_time_texture_t::State::DISAPPEARING)
            {
                std::memcpy(texture1_data->dataPointer(), stt.texture->dataPointer(), stt.texture->dataSize());
                texture1_data->dirty();

                stt.state = season_time_texture_t::State::DISAPPEARING;
            }

            mix_value->set(1.0f - static_cast<float>(stt.time_disappear_end.data() - sim_time.time.data())
                                  / static_cast<float>(stt.time_disappear_end.data() - stt.time_disappear_begin.data()));
            mix_value->dirty();
        }
        else
        {
            stt.state = season_time_texture_t::State::INACTIVE;
        }
    }*/
}

void NewSkybox::set_sun_direction(double azimuth_degrees, double altitude_degrees)
{
    // Поворот модели скайбокса в используемую систему координат,
    // и поворот для отрисовки солнца в той же стороне, где источник солнечного света
    transform->matrix = vsg::rotate(vsg::radians(90.0), vsg::dvec3{1.0, 0.0, 0.0}) *
                        vsg::rotate(vsg::radians(90.0 + azimuth_degrees), vsg::dvec3{0.0, -1.0, 0.0});

    // Выбор текстур по углу возвышения солнца над/под горизонтом
    if (std::abs(altitude_degrees) > 90.0)
    {
        return;
    }

    auto angle_in_interval = [](const double& begin,
                                const double& end,
                                const double& cur_angle,
                                const double& cur_angle_rise) -> float
    {
        if ((begin < end) == cur_angle_rise)
        {
            const float mix_value = (cur_angle - begin) / (end - begin);
            return std::clamp(mix_value, 0.0f, 1.0f);
        }
        return 0.0f;
    };

    float max1 = 0.0f;
    float max2 = 0.0f;
    int texture1_id = -1;
    int texture2_id = -1;
    int id = 0;
    for (texture_t& tt : textures)
    {
        // Проверям, с какой интенсивностью должна отображаться текстура
        float mix_value_appear =
            angle_in_interval(tt.angle_appear_begin, tt.angle_appear_end, altitude_degrees, is_sun_rise);
        float mix_value_disappear =
            angle_in_interval(tt.angle_disappear_begin, tt.angle_disappear_end, altitude_degrees, is_sun_rise);

        // Корректируем полученное значение в случае перехода через полдень
        if ((tt.angle_appear_begin < tt.angle_appear_end) &&
            (tt.angle_disappear_begin > tt.angle_disappear_end))
        {
            mix_value_appear += static_cast<float>(!is_sun_rise);
        }

        // Корректируем полученное значение в случае перехода через полночь
        if ((tt.angle_appear_begin > tt.angle_appear_end) &&
            (tt.angle_disappear_begin < tt.angle_disappear_end))
        {
            mix_value_appear += static_cast<float>(is_sun_rise);
        }

        tt.mix_value = mix_value_appear - mix_value_disappear;

        // Сохраняем id двух текстур с максимальным значением
        if (max1 < tt.mix_value)
        {
            if (max2 < max1)
            {
                max2 = max1;
                texture2_id = texture1_id;
            }
            max1 = tt.mix_value;
            texture1_id = id;
        }
        else
        {
            if (max2 < tt.mix_value)
            {
                max2 = tt.mix_value;
                texture2_id = id;
            }
        }
        ++id;
    }

    if (texture2_id < 0)
    {
        if (texture1_id < 0)
        {
            // Нет активных текстур
            for (auto tt : textures)
            {
                tt.use_id = 0;
            }
            return;
        }

        // Активна только одна текстура, передаём её в первую текстуру шейдера
        if (textures[texture1_id].use_id != 1)
        {
            std::memcpy(texture1_data->dataPointer(),
                        textures[texture1_id].texture->dataPointer(),
                        textures[texture1_id].texture->dataSize());
            texture1_data->dirty();
        }

        // Передаём в шейдер отображение первой текстуры полностью
        if (mix_value->value() != 0.0f)
        {
            mix_value->set(0.0f);
            mix_value->dirty();
        }

        for (auto tt : textures)
        {
            tt.use_id = 0;
        }
        textures[texture1_id].use_id = 1;
    }
    else
    {
        // Активны две текстуры, передаём их в шейдер
        if (textures[texture1_id].use_id != 1)
        {
            std::memcpy(texture1_data->dataPointer(),
                        textures[texture1_id].texture->dataPointer(),
                        textures[texture1_id].texture->dataSize());
            texture1_data->dirty();
        }

        if (textures[texture2_id].use_id != 2)
        {
            std::memcpy(texture2_data->dataPointer(),
                        textures[texture2_id].texture->dataPointer(),
                        textures[texture2_id].texture->dataSize());
            texture2_data->dirty();
        }

        // Передаём в шейдер смешение текстур
        const float sum = max1 + max2;
        const float mix = max2 / sum;
        constexpr float eps = 1.0f / 256.0f;
        if (std::abs(mix_value->value() - mix) > eps)
        {
            mix_value->set(mix);
            mix_value->dirty();
        }

        for (auto tt : textures)
        {
            tt.use_id = 0;
        }
        textures[texture1_id].use_id = 1;
        textures[texture2_id].use_id = 2;
    }
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
        for (auto& buffer_info : vid.arrays)
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

        ushort_indices = vid.indices->data->cast<vsg::ushortArray>();
        uint_indices = vid.indices->data->cast<vsg::uintArray>();
    }

    vsg::ref_ptr<vsg::Data> get_indices() const
    {
        if (ushort_indices)
        {
            return ushort_indices;
        }
        else
        {
            return uint_indices;
        }
    }

    std::size_t get_indices_size() const
    {
        if (ushort_indices)
        {
            return ushort_indices->size();
        }
        else
        {
            return uint_indices->size();
        }
    }

public:
    vsg::ref_ptr<vsg::vec3Array> positions;
    vsg::ref_ptr<vsg::vec2Array> tex_coords;
    vsg::ref_ptr<vsg::ushortArray> ushort_indices;
    vsg::ref_ptr<vsg::uintArray> uint_indices;
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
    draw_commands->addChild(vsg::BindIndexBuffer::create(fav.get_indices()));
    draw_commands->addChild(vsg::DrawIndexed::create(fav.get_indices_size(), 1, 0, 0, 0));

    transform = vsg::MatrixTransform::create();
    transform->matrix = vsg::rotate(vsg::radians(90.0), vsg::dvec3{1.0, 0.0, 0.0});

    transform->addChild(draw_commands);
    state_group->addChild(transform);
/*    //debug
    for (int i = -60; i < 60; ++i)
    {
        is_sun_rise = (i < 0);
        int d = is_sun_rise ? (30 + i) : (30 - i);
        set_sun_direction(0.0, d);

        std::string log = std::to_string(d) + " " + std::to_string(mix_value->value());
        for (texture_t& tt : textures)
        {
            log += " | " + tt.filename
                   + "(" + std::to_string(tt.use_id)
                   + ")=" + std::to_string(tt.mix_value);
        }
        LOG_INFO("%s", log.c_str());
    }
    exit(0);*/
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

        texture_t tt{};
        tt.texture = data;
        tt.filename = texture_filename.toStdString();

        if (!cfg.getDouble(sec_node, "SunAltitudeAngleAppearBegin", tt.angle_appear_begin) ||
            !cfg.getDouble(sec_node, "SunAltitudeAngleAppearEnd", tt.angle_appear_end) ||
            !cfg.getDouble(sec_node, "SunAltitudeAngleDisappearBegin", tt.angle_disappear_begin) ||
            !cfg.getDouble(sec_node, "SunAltitudeAngleDisappearEnd", tt.angle_disappear_end))
        {
            LOG_WARN("Failed to read sun altitude angles from config for skybox texture: %s", texture_path.c_str());

            sec_node = cfg.getNextSection();
            continue;
        }

        if ((std::abs(tt.angle_appear_begin) > 90.0) ||
            (std::abs(tt.angle_appear_end) > 90.0) ||
            (std::abs(tt.angle_disappear_begin) > 90.0) ||
            (std::abs(tt.angle_disappear_end) > 90.0) ||
            (tt.angle_appear_begin == tt.angle_appear_end) ||
            (tt.angle_disappear_begin == tt.angle_disappear_end))
        {
            LOG_WARN("Invalid sun altitude angles for skybox texture: %s", texture_path.c_str());

            sec_node = cfg.getNextSection();
            continue;
        }

        LOG_INFO("Loaded skybox texture: %s", texture_path.c_str());
        textures.emplace_back(tt);
        sec_node = cfg.getNextSection();
    }
}
