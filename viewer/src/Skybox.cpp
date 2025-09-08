#include "Skybox.h"

#include "CfgReader.h"
#include "filesystem.h"
#include "Logger.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Value.h>
#include <vsg/io/Options.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderModule.h>
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
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vulkan/vulkan_core.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Skybox::Skybox(const std::string& skybox_config_filepath,
               vsg::ref_ptr<vsg::Options> options)
{
    CfgReader cfg;
    if (cfg.load(skybox_config_filepath.c_str()))
    {
        init_model(cfg, options);
        init_textures(cfg, options);
    }
    else
    {
        LOG_WARN("Fail to open skybox config: %s", skybox_config_filepath.c_str());
    }
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
    std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> all_textures;
    all_textures.reserve(textures.size());
    for (const season_time_texture_t& stt : textures)
    {
        all_textures.emplace_back(stt.texture);
    }
    return all_textures;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::setDateTime(simulator_time_t sim_time)
{
    std::map<vsg::ref_ptr<vsg::ubvec4Array2D>, float> textures_and_weights;

    server_date_t date_begin;
    server_date_t date_end;
    for (const season_time_texture_t& stt : textures)
    {
        // Проверяем, что дата попадет в сезон применения текстуры
        date_begin = server_date_t(sim_time.date.year(),
                                   stt.date_season_begin.month,
                                   stt.date_season_begin.day);
        bool is_after_season_begin = (sim_time.date >= date_begin);

        date_end = server_date_t(sim_time.date.year(),
                                 stt.date_season_end.month,
                                 stt.date_season_end.day);
        bool is_before_season_end = (sim_time.date <= date_end);

        bool is_season = (date_begin > date_end) ?
            (is_after_season_begin || is_before_season_end) :
            (is_after_season_begin && is_before_season_end);

        if (!is_season)
        {
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
            textures_and_weights[stt.texture] = static_cast<float>(sim_time.time.data() - stt.time_appear_begin.data()) /
                                                static_cast<float>(stt.time_appear_end.data() - stt.time_appear_begin.data());
        }
        if (time_in_interval(sim_time.time, stt.time_appear_end, stt.time_disappear_begin))
        {
            textures_and_weights[stt.texture] = 1.0f;
        }
        if (time_in_interval(sim_time.time, stt.time_disappear_begin, stt.time_disappear_end))
        {
            textures_and_weights[stt.texture] = static_cast<float>(stt.time_disappear_end.data() - sim_time.time.data()) /
                                                static_cast<float>(stt.time_disappear_end.data() - stt.time_disappear_begin.data());
        }
    }
    setActiveTextures(textures_and_weights);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::setActiveTextures(std::map<vsg::ref_ptr<vsg::ubvec4Array2D>, float> textures_and_weights)
{
    bool repaint = false;

    for (auto& [texture, weight] : textures_and_weights)
    {
        // Добавляем текстуры, которые не были активны
        if (!active_textures_and_weights.count(texture))
        {
            active_textures_and_weights[texture] = weight;
            repaint = true;
            continue;
        }
    }

    for (auto it = active_textures_and_weights.begin(); it != active_textures_and_weights.end();)
    {
        auto texture = it->first;
        float& weight = it->second;

        // Убираем текстуры, которые стали не активны
        if (!textures_and_weights.count(texture))
        {
            it = active_textures_and_weights.erase(it);
            repaint = true;
            continue;
        }

        ++it;

        constexpr float eps = 1.0f / 256.0f;
        if (std::abs(textures_and_weights[texture] - weight) > eps)
        {
            weight = textures_and_weights[texture];
            repaint = true;
            continue;
        }
    }

    if (repaint)
    {
        update_skybox();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::update_skybox()
{
    float sum_weights = 0.0f;

    // Собираем итераторы по пикселям активных текстур
    std::vector<std::pair<vsg::stride_iterator<vsg::ubvec4>, float>> active_pixels;
    active_pixels.reserve(active_textures_and_weights.size());

    for (const auto& [texture, weight] : active_textures_and_weights)
    {
        if (weight > 1.0e-5f)
        {
            active_pixels.emplace_back(texture->begin(), weight);
            sum_weights += weight;
        }
    }

    // Попиксельно перекрашиваем основную текстуру
    vsg::stride_iterator<vsg::ubvec4> texture_pixel = texture->begin();
    while (texture_pixel != texture->end())
    {
        // Закрашиваем чёрным
        texture_pixel->set(0, 0, 0, 255);
        for (auto& [pixel, weight] : active_pixels)
        {
            // Добавляем цвет из каждой активной текстуры
            const float k = (sum_weights > 1.0f) ? (weight / sum_weights) : weight;
            const vsg::ubvec4 color(k * pixel->r, k * pixel->g, k * pixel->b, 0);
            *texture_pixel += color;

            ++pixel;
        }
        ++texture_pixel;
    }
    texture->dirty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::init_model(CfgReader &cfg, vsg::ref_ptr<vsg::Options> options)
{


    vsg::ref_ptr<vsg::Options> skybox_options = options ? vsg::Options::create(*options) : vsg::Options::create();
    skybox_options->sharedObjects = {};

    // За основу берём встроенные комплекты вершинного и фрагментного шейдера
    skybox_options->shaderSets["flat"] = vsg::createFlatShadedShaderSet();
    skybox_options->shaderSets["pbr"] = vsg::createPhysicsBasedRenderingShaderSet();
    skybox_options->shaderSets["phong"] = vsg::createPhongShaderSet();

    // Очищаем все встроенные сохранённые варианты настроек
    skybox_options->shaderSets["flat"]->variants.clear();
    skybox_options->shaderSets["pbr"]->variants.clear();
    skybox_options->shaderSets["phong"]->variants.clear();

    auto vertexInputState = vsg::VertexInputState::create();
    auto inputAssemblyState = vsg::InputAssemblyState::create();
    auto rasterizationState = vsg::RasterizationState::create();
    auto colorBlendState = vsg::ColorBlendState::create();
    auto depthStencilState = vsg::DepthStencilState::create();
    auto multisampleState = vsg::MultisampleState::create();

    // Отключаем проверку на глубину сцены
    depthStencilState->depthTestEnable = VK_FALSE;

    vsg::GraphicsPipelineStates defaultGraphicsPipelineStates = {
        vertexInputState,
        inputAssemblyState,
        rasterizationState,
        colorBlendState,
        depthStencilState,
        multisampleState
    };

    skybox_options->shaderSets["flat"]->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    skybox_options->shaderSets["pbr"]->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    skybox_options->shaderSets["phong"]->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;

    FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    // Загружаем свой вариант вершинного шейдера вместо встроенного
    const std::string shader_vert_path = shaders_dir_path + fs.separator() + "skybox_vert.vert";
    const vsg::ref_ptr<vsg::ShaderStage> shader_vert_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", shader_vert_path, skybox_options);
    if (shader_vert_stage)
    {
        LOG_INFO("Loaded vertex shader for skybox: %s", shader_vert_path.c_str());
        skybox_options->shaderSets["flat"]->stages.front() = shader_vert_stage;
        skybox_options->shaderSets["pbr"]->stages.front() = shader_vert_stage;
        skybox_options->shaderSets["phong"]->stages.front() = shader_vert_stage;
    }
    else
    {
        LOG_WARN("Fail to load vertex shader for skybox: %s", shader_vert_path.c_str());
        LOG_INFO("Using default vertex shader");
    }

    // Загружаем свой вариант фрагментного шейдера вместо встроенного
    const std::string shader_frag_path = shaders_dir_path + fs.separator() + "skybox_frag.frag";
    const vsg::ref_ptr<vsg::ShaderStage> shader_frag_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", shader_frag_path, skybox_options);
    if (shader_frag_stage)
    {
        LOG_INFO("Loaded fragment shader for skybox: %s", shader_frag_path.c_str());
        skybox_options->shaderSets["flat"]->stages.back() = shader_frag_stage;
        skybox_options->shaderSets["pbr"]->stages.back() = shader_frag_stage;
        skybox_options->shaderSets["phong"]->stages.back() = shader_frag_stage;
    }
    else
    {
        LOG_WARN("Fail to load fragment shader for skybox: %s", shader_frag_path.c_str());
        LOG_INFO("Using default fragment shader");
    }

    // Загружаем модель скайбокса
    QString model_filename = "sky.gltf";
    cfg.getString("Model", "Filename", model_filename);

    std::string model_path = fs.getDataDir();
    model_path = fs.combinePath(model_path, "models");
    model_path = fs.combinePath(model_path, "default-objects");
    model_path = fs.combinePath(model_path, model_filename.toStdString());

    if (!vsg::fileExists(model_path))
    {
        LOG_WARN("Fail to find skybox file: %s", model_path.c_str());
        return;
    }

    const vsg::ref_ptr<vsg::Object> loaded = vsg::read(model_path, skybox_options);
    node = loaded.cast<vsg::Node>();
    if (!node)
    {
        LOG_WARN("Fail to load skybox model from file: %s", model_path.c_str());

        vsg::ref_ptr<vsg::ReadError> error = loaded.cast<vsg::ReadError>();
        if (error)
        {
            LOG_WARN(error->message.c_str());
        }

        return;
    }

    LOG_INFO("Loaded skybox model from file: %s", model_path.c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Skybox::init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options)
{
    if (!node)
    {
        return;
    }

    // Находим дефолтную текстуру в модели
    struct findTexture : public vsg::Visitor
    {
        vsg::ref_ptr<vsg::ubvec4Array2D> texture;

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
    FileSystem& fs = FileSystem::getInstance();
    std::string textures_dir_path = fs.getDataDir();
    textures_dir_path = fs.combinePath(textures_dir_path, "models");
    textures_dir_path = fs.combinePath(textures_dir_path, "default-objects");
    textures_dir_path = fs.combinePath(textures_dir_path, "textures");

    // Читаем из конфига имена файлов текстур и их сезон, время суток
    QDomNode secNode = cfg.getFirstSection("Texture");
    while (!secNode.isNull())
    {
        QString texture_filename = "sky_day.bmp";
        cfg.getString(secNode, "Filename", texture_filename);
        std::string texture_path = fs.combinePath(textures_dir_path, texture_filename.toStdString());

        // Ищем файл текстуры
        if (!vsg::fileExists(texture_path))
        {
            LOG_WARN("Fail to find skybox texture file: %s", texture_path.c_str());

            secNode = cfg.getNextSection();
            continue;
        }

        // Загружаем файл текстуры
        vsg::ref_ptr<vsg::Object> loaded = vsg::read(texture_path, options);
        vsg::ref_ptr<vsg::ubvec4Array2D> data = loaded.cast<vsg::ubvec4Array2D>();
        if (!data)
        {
            LOG_WARN("Fail to load skybox texture from file: %s", texture_path.c_str());

            vsg::ref_ptr<vsg::ReadError> error = loaded.cast<vsg::ReadError>();
            if (error)
                LOG_WARN(error->message.c_str());

            secNode = cfg.getNextSection();
            continue;
        }

        // Проверяем, что новая текстура совпадает по размеру с дефолтной
        if ((data->width() != texture->width()) || (data->height() != texture->height()))
        {
            LOG_WARN("Fail to apply skybox texture from file: %s", texture_path.c_str());

            secNode = cfg.getNextSection();
            continue;
        }

        LOG_INFO("Loaded skybox texture from file: %s", texture_path.c_str());
        season_time_texture_t stt {};
        stt.texture = data;
        stt.filename = texture_filename.toStdString();

        auto read_season_date = [](CfgReader& cfg, QDomNode& secNode, const char* section,
                                   season_time_texture_t::season_date_t& sd) -> bool
        {
            QString tmp;
            if (!cfg.getString(secNode, section, tmp))
            {
                return false;
            }

            QStringList tokens = tmp.split("-");
            if (tokens.size() != 2)
            {
                return false;
            }

            sd.month = tokens[0].toInt();
            if ((sd.month < 1) ||  (sd.month > 12))
            {
                return false;
            }

            sd.day = tokens[1].toInt();
            if ((sd.day < 1) ||  (sd.day > 31))
            {
                return false;
            }

            return true;
        };

        auto read_time = [](CfgReader& cfg, QDomNode& secNode, const char* section,
                            server_time_t& st) -> bool
        {
            QString tmp;
            if (!cfg.getString(secNode, section, tmp))
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

        if (!read_season_date(cfg, secNode, "SeasonBegin", stt.date_season_begin) ||
            !read_season_date(cfg, secNode, "SeasonEnd", stt.date_season_end) ||
            !read_time(cfg, secNode, "TimeAppearBegin", stt.time_appear_begin) ||
            !read_time(cfg, secNode, "TimeAppearEnd", stt.time_appear_end) ||
            !read_time(cfg, secNode, "TimeDisappearBegin", stt.time_disappear_begin) ||
            !read_time(cfg, secNode, "TimeDisappearEnd", stt.time_disappear_end))
        {
            LOG_WARN("Fail to read season or time from config for skybox texture: %s", texture_path.c_str());

            secNode = cfg.getNextSection();
            continue;
        }


        int through_midnight_count = (stt.time_appear_begin > stt.time_appear_end)
                                     + (stt.time_appear_end > stt.time_disappear_begin)
                                     + (stt.time_disappear_begin > stt.time_disappear_end);
        if (through_midnight_count > 1)
        {
            LOG_WARN("Invalid time for skybox texture: %s", texture_path.c_str());

            secNode = cfg.getNextSection();
            continue;
        }

        textures.emplace_back(stt);
        secNode = cfg.getNextSection();
    }

    // Указываем vsg, что возможна подмена текстуры скайбокса
    if (!textures.empty())
    {
        texture->properties.dataVariance = vsg::DYNAMIC_DATA;
    }
}
