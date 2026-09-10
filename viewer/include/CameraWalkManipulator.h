#ifndef CAMERA_WALK_MANIPULATOR_H
#define CAMERA_WALK_MANIPULATOR_H

#include "CameraAbstract.h"

#include <set>
#include <vector>

#include <player-controller.h>
#include <collision-world.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CameraWalkManipulator final : public CameraAbstract
{
public:

    /// world - мир коллизий маршрута (может быть nullptr: бесконечный
    /// пол на z = 0 - для отладки и маршрутов без коллизий)
    CameraWalkManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                          vsg::ref_ptr<vsg::Camera> camera,
                          settings_t& settings,
                          PlayerController& player,
                          collision::CollisionWorld* world);

    /// Активация режима: спавн игрока в точке (выход из кабины /
    /// переключение в пешую камеру)
    void activate(const vsg::dvec3& spawn_pos);

    /// Направление взгляда игрока (для PlayerController), рад
    double getYaw() const;

    /// Сесть на сиденье (E): камера фиксируется в точке, обзор мышью
    /// остаётся, движение отключается. eye_h - высота глаз сидя
    void sitDown(const vsg::dvec3& seat_world_pos, double eye_h);

    /// Встать (E повторно): телепорт рядом с точкой стоя
    void standUp(const vsg::dvec3& stand_world_pos);

    bool isSeated() const;

    void resetView() override;
    void returnView() override;
    void mouseWheelEvent(vsg::vec3 delta) override;
    void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) override;
    void touchZoomEvent(double zoomLevel) override;
    void frameEvent(double dt) override;

    void keyboardPressEvent(vsg::KeySymbol key, bool pressed) override;

    /// Запустить анимацию входа в кабину (path: от глаз игрока до
    /// итоговой точки камеры в кабине)
    void startBoarding(const std::vector<vsg::dvec3>& path, double duration_s);

    void cancelBoarding();

    bool isBoarding() const;

    /// Высота глаз игрока (для построения пути анимации)
    double playerEyeHeight() const;

    /// Позиция игрока (центр коллизии, мировые координаты)
    const collision::Vec3f& getPlayerPosition() const;

    /// Сдвинуть игрока на дельту (перенос движущимся локомотивом)
    void shiftPlayer(const vsg::dvec3& delta);

    /// Переместить игрока в точку (центр коллизии), камера следует
    void teleportPlayer(const vsg::dvec3& world);

    /// Сброс собственного состояния клавиш (потеря фокуса окна)
    void clearKeys();

private:

    void updateView();

    /// Обновить камеру в сидячем режиме (кабина)
    void updateViewSeated();

    PlayerController& _player;
    collision::CollisionWorld* _world = nullptr;

    /// Обзор: рысканье/тангаж (рад). Управляется мышью (при зажатой
    /// кнопке - события движения мыши приходят только при фокусе)
    double _yaw = 0.0;
    double _pitch = 0.0;

    /// Покачивание головы при ходьбе (ТЗ "walking", п.11): фаза шагов
    /// и амплитуда с плавным затуханием при остановке
    double _bob_phase = 0.0;
    double _bob_amplitude = 0.0;

    /// FOV при беге (п.12): текущее значение стремится к целевому
    /// (fovy + прирост при спринте) со скоростью fov_speed
    double _fov_current = 64.0;

    /// Проседание камеры при приземлении (п.13): зависит от скорости
    /// удара, восстанавливается экспоненциально
    double _landing_dip = 0.0;

    /// Посадка на сиденье (E): камера фиксирована, ходьба выключена
    bool _seated = false;

    /// Собственное состояние клавиш: событие press/release приходит в
    /// манипулятор напрямую и не зависит от особенностей vsg::Keyboard
    std::set<int> _own_pressed;
    vsg::dvec3 _seat_pos = {0.0, 0.0, 0.0};
    double _seat_eye_h = 1.15;

    /// Подъём по ступеням при входе в кабину: камера движется по
    /// кривой (глаза игрока -> ступени -> порог -> место машиниста)
    /// с покачиванием шагов; физика ходьбы на время отключается
    std::vector<vsg::dvec3> boarding_curve_;
    double boarding_t_ = 0.0;
    double boarding_duration_ = 1.5;
    double boarding_z_rise_ = 0.0;
    bool boarding_active_ = false;
};

#endif // CAMERA_WALK_MANIPULATOR_H