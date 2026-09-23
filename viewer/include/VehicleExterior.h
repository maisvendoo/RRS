#pragma once
#ifndef VEHICLE_EXTERIOR_H
#define VEHICLE_EXTERIOR_H

#include <vsg/core/Object.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>

class SoundManager;
class IOController;
//class AnimatedPagedLOD;       // Forward declare не работает,
#include "AnimatedPagedLOD.h"   // VehiclesHandler ругается на incomplete use

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehicleExterior final
{
public:
    VehicleExterior() = default;

    vsg::ref_ptr<vsg::CullNode> cullnode = vsg::CullNode::create();
    vsg::ref_ptr<vsg::MatrixTransform> transform = vsg::MatrixTransform::create();
    vsg::dvec3  position = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  orth = vsg::dvec3(0.0, 1.0, 0.0);
    vsg::dvec3  up = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dvec3  right = vsg::dvec3(1.0, 0.0, 0.0);
    vsg::dvec3  velocity = vsg::dvec3(0.0, 0.0, 0.0);
    std::vector<vsg::dvec3>  driver_pos = {vsg::dvec3(0.0, 0.0, 0.0)};
    std::vector<double>  driver_dir = {0};

    /// Точки выхода из кабин (пешая ходьба, ТЗ "walking"): спавн игрока
    /// при выходе; при отсутствии <ExitPos> - у двери, сдвиг от DriverPos
    std::vector<vsg::dvec3>  exit_pos = {};
    std::vector<double>  exit_dir = {};

    /// Сиденье помощника в кабинах (посадка по E, ТЗ ходьба)
    std::vector<vsg::dvec3>  assistant_pos = {};
    int         train_id = 0;
    int         orientation = 1;
    int         prev_vehicle = -1;
    int         next_vehicle = -1;

    std::vector<size_t> sounds_id = {};
    std::vector<vsg::ref_ptr<AnimatedPagedLOD>> animated_nodes;

    /// Интерактивные органы кабины из [CabElement] конфига ПС

    /// Текущее значение анимационного сигнала (состояние органа),
    /// -1 - сигнала нет. Для тултипа Alt-режима
    float getCabSignal(int signal_id) const;

    /// СЫРОЕ значение сигнала от сервера (без анимационного
    /// сглаживания): мгновенное состояние органа для клика, -1 - нет
    float getRawSignal(int signal_id) const;

    /// Последний набор аналоговых сигналов сервера
    const std::vector<float>* last_server_signals = nullptr;

    vsg::dvec3  saved_cabine_cam_shift = vsg::dvec3(0.0, 0.0, 0.0);
    double      saved_cabine_cam_right = 0.0;
    double      saved_cabine_cam_up = 0.0;
    double      saved_cabine_cam_fov = 64.0;

    /// Реакция камеры от физики (ТЗ "Физическая реакция машиниста"):
    /// смещение головы в локальных осях ПЕ (X - продольное, Y - поперечное,
    /// Z - вертикальное), м; и наклоны (крен/тангаж), рад
    vsg::dvec3  cam_motion_offset = vsg::dvec3(0.0, 0.0, 0.0);
    double      cam_motion_roll = 0.0;
    double      cam_motion_pitch = 0.0;

    /// Заданный индекс кабины
    size_t current_cabine_idx = 0;

    /// Фактический индекс кабины
    size_t controlled_cabine_idx = 0;

    /// Контроллер ввода/вывода
    std::vector<IOController *> io_controls = {nullptr};

    void step(float t, float dt);
    void step(float t, float dt, std::vector<float> *server_signals);

    bool loadVehicle(const std::string& cfg_dir,
                     const std::string& cfg_file,
                     SoundManager *sm,
                     vsg::ref_ptr<vsg::Options> options);

    /// Мировая позиция локальной точки ПЕ (x - вправо, y - вперёд,
    /// z - вверх от центра ПЕ) по текущим интерполированным осям
    vsg::dvec3 worldFromLocal(const vsg::dvec3& local) const;

private:

    /// Загрузка положения камеры в кабинах
    bool load_cabine_positions(const std::string& cfg_path, CfgReader& cfg);

    /// Загрузка звуков
    bool load_sounds(const std::string& cfg_path, CfgReader& cfg, SoundManager* sm);

    /// Загрузка моделей
    bool load_models(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

    /// Методы под старый формат конфига с параметрами <ExtModelName> и <CabineModel>
    bool load_body_model(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    bool load_cabine_model(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

    /// Загрузка модуля ввода/вывода
    bool load_io_controller_module(const std::string &cfg_path, CfgReader &cfg);
};

#endif // VEHICLE_EXTERIOR_H
