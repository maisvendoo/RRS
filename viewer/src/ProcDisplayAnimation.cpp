#include "ProcDisplayAnimation.h"

#include "CfgReader.h"
#include "Logger.h"
#include "display-types.h"
#include "filesystem.h"

#include <QApplication>
#include <QThread>
#include <QPainter>

#include <sstream>
#include <vsg/maths/vec4.h>
#include <vsg/state/Image.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcDisplayAnimation::ProcDisplayAnimation(vsg::ref_ptr<vsg::Image> in_image_color,
                                           vsg::ref_ptr<vsg::Image> in_image_emissive,
                                           vsg::ref_ptr<vsg::PbrMaterialValue> in_material_value)
    : Inherit()
    , image_color(in_image_color)
    , image_emissive(in_image_emissive)
    , material_value(in_material_value)
    , base_color(in_material_value->value().baseColorFactor)
    , emission_color(in_material_value->value().emissiveFactor)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::size_t ProcDisplayAnimation::getSignalID() const
{
    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcDisplayAnimation::anim_step(float t, float dt)
{
    if (display)
    {
        // Обновляем сигналы внутри дисплейного модуля
        if (server_signals && (server_signals != prev_signals))
        {
            display_signals_t display_signals;
            std::copy(server_signals->begin(), server_signals->end(), display_signals.begin());
            display->setInputSignals(display_signals);
            prev_signals = server_signals;
        }

        // Обновляем дисплейный модуль
        display->update(t, dt);

        {
            QPainter painter(&qimage);
            display->render(&painter);
            painter.end();
        }

        // Указываем VSG обновить текстуру, data уже указывает на пиксели в qimage
        if (is_color_repaint)
        {
            image_color->data->dirty();
        }
        if (is_emissive_repaint)
        {
            image_emissive->data->dirty();
        }
    }

    // Яркость дисплея
    if (is_fixed_signal)
    {
        return;
    }

    float server_signal = 0.0f;
    if (server_signals && (signal_id >= 0) && (static_cast<std::size_t>(signal_id) < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id];
    }

    float delta = server_signal - cur_signal;
    if (abs(delta) > 1e-5f)
    {
        cur_signal += delta * fmin(duration * dt, 1.0f);
        update(cur_signal);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcDisplayAnimation::update(float current_signal)
{
    if (!keypoints.empty())
    {
        const float color_factor = interpolate(current_signal);
        vsg::vec4 new_color = base_color * color_factor;
        new_color.a = 1.0f;
        material_value->value().baseColorFactor = new_color;
    }

    vsg::vec4 new_emission_color = emission_color * current_signal;
    new_emission_color.a = 1.0f;
    material_value->value().emissiveFactor = new_emission_color;
    material_value->dirty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcDisplayAnimation::load_config(CfgReader &cfg)
{
    if (!material_value)
    {
        LOG_WARN("Fail to create display animation: no material value");
        return false;
    }

    QString sec_name = "Display";

    if (cfg.getBool(sec_name, "RepaintColor", is_color_repaint))
    {
        if (is_color_repaint && (!image_color))
        {
            LOG_WARN("Fail to configure display animation: no color texture to repaint");
            return false;
        }
    }

    if (cfg.getBool(sec_name, "RepaintEmissive", is_emissive_repaint))
    {
        if (is_emissive_repaint && (!image_emissive))
        {
            LOG_WARN("Fail to configure display animation: no emissive texture to repaint");
            return false;
        }
    }

    if (!(is_color_repaint || is_emissive_repaint))
    {
        LOG_WARN("Fail to configure display animation: no repaint needed");
        return false;
    }

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
    {
        signal_id = tmp_int;
    }

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
    {
        duration = tmp_dbl;
    }

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    QString tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "EmissionColor", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color.r >> emission_color.g >> emission_color.b;
    }

    tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        vsg::vec4 config_color_limit = {1.0f, 1.0f, 1.0f, 1.0f};
        ss >> config_color_limit.r >> config_color_limit.g >> config_color_limit.b;
        base_color *= config_color_limit;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string module_path = fs.getModulesDir();
    std::string cfgdir_path = fs.getConfigDir();

    tmp_qstr = "";
    if (cfg.getString(sec_name, "ModuleDir", tmp_qstr))
    {
        module_path = fs.combinePath(module_path, tmp_qstr.toStdString());
    }

    tmp_qstr = "";
    if (cfg.getString(sec_name, "Module", tmp_qstr))
    {
        module_path = fs.combinePath(module_path, tmp_qstr.toStdString());
    }

    tmp_qstr = "";
    if (cfg.getString(sec_name, "ModuleConfigDir", tmp_qstr))
    {
        cfgdir_path = fs.combinePath(cfgdir_path, tmp_qstr.toStdString());
    }

    tmp_qstr = "";
    if (cfg.getString(sec_name, "ModuleConfigSubDir", tmp_qstr))
    {
        cfgdir_path = fs.combinePath(cfgdir_path, tmp_qstr.toStdString());
    }

    display = loadDisplay(module_path.c_str());

    if (!display)
    {
        LOG_WARN("Fail to load display module: %s", module_path.c_str());
        return false;
    }

    display->setConfigDir(QString(cfgdir_path.c_str()));
    display->init();
    LOG_INFO("Config's directory %s for loaded display module %s ", cfgdir_path.c_str(), module_path.c_str());

    // Рендер дисплея, чтобы перерисовать текстуру на нужный размер до компиляции модели
    qimage = QImage(display->size(), QImage::Format_RGBA8888_Premultiplied);
    qimage.fill(Qt::black);
    {
        QPainter painter(&qimage);
        display->render(&painter);
        painter.end();
    }

    // Задаём текстуре пиксели экземпляра QImage как источник данных
    vsg::ubvec4* qimage_pixels = reinterpret_cast<vsg::ubvec4*>(qimage.bits());
    if (is_color_repaint)
    {
        image_color->data = vsg::ubvec4Array2D::create(qimage.width(), qimage.height(), qimage_pixels, image_color->data->properties);
        image_color->data->properties.dataVariance = vsg::DYNAMIC_DATA;

        // Задаём текстуре новый размер
        uint32_t width = image_color->data->width() * image_color->data->properties.blockWidth;
        uint32_t height = image_color->data->height() * image_color->data->properties.blockHeight;
        uint32_t depth = image_color->data->depth() * image_color->data->properties.blockDepth;
        image_color->extent = VkExtent3D{width, height, depth};
    }
    if (is_emissive_repaint)
    {
        image_emissive->data = vsg::ubvec4Array2D::create(qimage.width(), qimage.height(), qimage_pixels, image_emissive->data->properties);
        image_emissive->data->properties.dataVariance = vsg::DYNAMIC_DATA;

        // Задаём текстуре новый размер
        uint32_t width = image_emissive->data->width() * image_emissive->data->properties.blockWidth;
        uint32_t height = image_emissive->data->height() * image_emissive->data->properties.blockHeight;
        uint32_t depth = image_emissive->data->depth() * image_emissive->data->properties.blockDepth;
        image_emissive->extent = VkExtent3D{width, height, depth};
    }

    // Обновляем яркость дисплея
    material_value->properties.dataVariance = vsg::DYNAMIC_DATA;
    update(cur_signal);
    return true;
}
