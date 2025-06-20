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
ProcDisplayAnimation::ProcDisplayAnimation(
    vsg::ref_ptr<vsg::Image> in_image_data,
    vsg::ref_ptr<vsg::PbrMaterialValue> in_material_data
)
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

        // Рендер дисплея
        qimage.fill(Qt::blue);
        if (QThread::currentThread() != qApp->thread())
        {
            QMetaObject::invokeMethod(qApp, [&]() {
                    QPainter painter(&qimage);
                    display->render(&painter);
                    painter.end();
                }, Qt::BlockingQueuedConnection);
        }
        else
        {
            QPainter painter(&qimage);
            display->render(&painter);
            painter.end();
        }

        // Указываем VSG обновить текстуру, data уже указывает на пиксели в qimage
        image_data->data->dirty();
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
    std::string module_dir;
    std::string module_name;
    std::string module_path;

    FileSystem& fs = FileSystem::getInstance();

    QString sec_name = "Display";

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

    module_path = fs.combinePath(fs.getModulesDir(), module_dir);
    module_path = fs.combinePath(module_path, module_name);

    // Загрузка дисплея
    if (QThread::currentThread() != qApp->thread())
    {
        QMetaObject::invokeMethod(qApp, [&]() {
            display = loadDisplay(module_path.c_str());
        }, Qt::BlockingQueuedConnection);
    }
    else
    {
        display = loadDisplay(module_path.c_str());
    }

    if (!display)
    {
        LOG_WARN("Fail to load display module: %s", module_path.c_str());
        return false;
    }
    LOG_INFO("Loaded display module: %s", module_path.c_str());

    // Передать каким-то образом папку с конфигом ПЕ из VehicleHandler или VehicleExterior?
    // Или конфигурировать полностью из конфига с анимацией?
    // TODO // display->setConfigDir(QString(""));

    // Инициализация дисплейного модуля
    if (QThread::currentThread() != qApp->thread())
    {
        QMetaObject::invokeMethod(qApp, [&]() {
            display->init();
        }, Qt::BlockingQueuedConnection);
    }
    else
    {
        display->init();
    }

    // Рендер дисплея, чтобы перерисовать текстуру на нужный размер до компиляции модели
    qimage = QImage(display->size(), QImage::Format_RGBA8888_Premultiplied);
    qimage.fill(Qt::blue);
    if (QThread::currentThread() != qApp->thread())
    {
        QMetaObject::invokeMethod(qApp, [&]() {
            QPainter painter(&qimage);
            display->render(&painter);
            painter.end();
        }, Qt::BlockingQueuedConnection);
    }
    else
    {
        QPainter painter(&qimage);
        display->render(&painter);
        painter.end();
    }

    // Задаём текстуре пиксели экземпляра QImage как источник данных
    vsg::ref_ptr<vsg::ubvec4Array2D> texture_pixels = image_data->data.cast<vsg::ubvec4Array2D>();
    vsg::ubvec4* qimage_pixels = reinterpret_cast<vsg::ubvec4*>(qimage.bits());
    texture_pixels->assign(qimage.width(), qimage.height(), qimage_pixels, texture_pixels->properties);

    // Задаём текстуре новый размер
    uint32_t width = image_data->data->width() * image_data->data->properties.blockWidth;
    uint32_t height = image_data->data->height() * image_data->data->properties.blockHeight;
    uint32_t depth = image_data->data->depth() * image_data->data->properties.blockDepth;
    image_data->extent = VkExtent3D{width, height, depth};

    image_data->data->properties.dataVariance = vsg::DYNAMIC_DATA;

    // Обновляем яркость дисплея
    update(cur_signal);
    return true;
}
