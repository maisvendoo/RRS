#include "Skybox.h"

#include "CfgReader.h"
#include "filesystem.h"
#include "Logger.h"

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

        bool is_season = (date_end > date_begin) ?
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

    for (auto& [texture, weight] : active_textures_and_weights)
    {
        // Убираем текстуры, которые стали не активны
        if (!textures_and_weights.count(texture))
        {
            active_textures_and_weights.erase(texture);
            repaint = true;
            continue;
        }

        constexpr float eps = 1.0f / 256.0f;
        if (abs(textures_and_weights[texture] - weight) > eps)
        {
            active_textures_and_weights[texture] = textures_and_weights[texture];
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
    // Собираем итераторы по пикселям активных текстур
    float sum_weights = 0.0f;
    std::vector<std::pair<vsg::stride_iterator<vsg::ubvec4>, float>> active_pixels;
    for (const auto& [texture, weight] : active_textures_and_weights)
    {
        if (weight > 1.0e-5f)
        {
            active_pixels.push_back({texture->begin(), weight});
            sum_weights += weight;
        }
    }

    // Попиксельно перекрашиваем основную текстуру
    vsg::stride_iterator<vsg::ubvec4> texture_pixel = texture->begin();
    while (texture_pixel != texture->end())
    {
        // Закрашиваем чёрным
        *texture_pixel = vsg::ubvec4{0, 0, 0, 255};
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
    QString model_filename = "sky.gltf";
    cfg.getString("Model", "Filename", model_filename);

    FileSystem& fs = FileSystem::getInstance();
    std::string model_path = fs.getDataDir();
    model_path = fs.combinePath(model_path, "models");
    model_path = fs.combinePath(model_path, "default-objects");
    model_path = fs.combinePath(model_path, model_filename.toStdString());

    if (!vsg::fileExists(model_path))
    {
        LOG_WARN("Fail to find skybox file: %s", model_path.c_str());
        return;
    }

    const vsg::ref_ptr<vsg::Object> loaded = vsg::read(model_path, options);
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

    const std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    // Загружаем свой вариант вершинного шейдера вместо встроенного
    const std::string shader_vert_path = shaders_dir_path + fs.separator() + "skybox_vert.vert";
    const vsg::ref_ptr<vsg::ShaderStage> shader_vert_stage =
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


        int trough_midhight_count = (stt.time_appear_begin > stt.time_appear_end) +
                                    (stt.time_appear_end > stt.time_disappear_begin) +
                                    (stt.time_disappear_begin > stt.time_disappear_end);
        if (trough_midhight_count > 1)
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
