#include "ProcDisplayAnimation.h"

#include "CfgReader.h"
#include "Logger.h"
#include "filesystem.h"

#include <QApplication>
#include <QPainter>

#include <sstream>
#include <vsg/state/Image.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcDisplayAnimation::ProcDisplayAnimation(vsg::ref_ptr<vsg::Image> in_image_data,
                                           vsg::ref_ptr<vsg::PbrMaterialValue> in_material_data)
    : Inherit()
    , image_data(in_image_data)
    , material_value(in_material_data)
    , base_color(in_material_data->value().baseColorFactor)
    , emission_color(in_material_data->value().emissiveFactor)
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
    if (!display)
    {
        return;
    }

    QImage image(display->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // QMetaObject::invokeMethod(qApp, [&]() {
        QPainter painter(&image);
        display->render(&painter);
        painter.end();
    // }, Qt::BlockingQueuedConnection);

    // for (int i = 0; i < image.width() * image.height(); ++i)
    // {
    //     std::cout << image.bits()[i] << std::endl;
    // }
    image.save("test.png");

    vsg::ref_ptr<vsg::Data> vsgData;
    if (image.format() == QImage::Format_ARGB32_Premultiplied)
    {
        vsgData = vsg::ubvec4Array2D::create(
            image.width(),
            image.height(),
            reinterpret_cast<vsg::ubvec4*>(image.bits()),
            vsg::Data::Layout{VK_FORMAT_R8G8B8A8_UNORM}
        );
    }
    else
    {
        QImage converted = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        vsgData = vsg::ubvec4Array2D::create(
            converted.width(),
            converted.height(),
            reinterpret_cast<vsg::ubvec4*>(converted.bits()),
            vsg::Data::Layout{VK_FORMAT_R8G8B8A8_UNORM}
        );
    }

    auto pixels = image_data->data.cast<vsg::ubvec4Array2D>();
    auto new_pixels = vsgData.cast<vsg::ubvec4Array2D>();

    if (pixels->size() != new_pixels->size())
    {
        image_data->data = vsgData;
        image_data->data->properties.dataVariance = vsg::DYNAMIC_DATA;
        image_data->data->dirty();
    }
    else
    {
        for (std::size_t i = 0; i < pixels->size(); ++i)
        {
            pixels->at(i) = new_pixels->at(i);
            image_data->data->dirty();
        }
    }

    // if ((sin(t) > 0.0f))
    // {
    //     if (!prev_sin_t_positive)
    //     {
    //         prev_sin_t_positive = true;

    //         vsg::ref_ptr<vsg::ubvec4Array2D> pixels = image_data->data.cast<vsg::ubvec4Array2D>();
    //         for (auto it = pixels->begin(); it != pixels->end(); ++it)
    //             (*it) = vsg::ubvec4(255, 0, 0, 255); // Красный
    //         image_data->data->dirty();
    //     }
    // }
    // else
    // {
    //     if (prev_sin_t_positive)
    //     {
    //         prev_sin_t_positive = false;

    //         vsg::ref_ptr<vsg::ubvec4Array2D> pixels = image_data->data.cast<vsg::ubvec4Array2D>();
    //         for (auto it = pixels->begin(); it != pixels->end(); ++it)
    //             (*it) = vsg::ubvec4(0, 255, 0, 255); // Зелёный
    //         image_data->data->dirty();
    //     }
    // }

    // Яркость дисплея
    if (is_fixed_signal)
        return;

    float server_signal = 0.0f;
    if (server_signals && (signal_id >= 0) && (signal_id < server_signals->size()))
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
        float color_factor = interpolate(current_signal);
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
    FileSystem& fs = FileSystem::getInstance();

    QString sec_name = "Display";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

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

    tmp_qstr = "";
    if (cfg.getString(sec_name, "ModuleDir", tmp_qstr))
    {
        module_dir = tmp_qstr.toStdString();
    }

    tmp_qstr = "";
    if (cfg.getString(sec_name, "Module", tmp_qstr))
    {
        module_name = tmp_qstr.toStdString();
    }

    module_dir = fs.combinePath(fs.getModulesDir(), module_dir);
    module_name = fs.combinePath(module_dir, module_name);

    display = loadDisplay(module_name.c_str());
    if (!display)
    {
        LOG_WARN("Module %s is not found", module_name.c_str());
        return false;
    }

    update(cur_signal);
    return true;
}
