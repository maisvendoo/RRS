#include "CameraWalkManipulator.h"


#include <Logger.h>

#include "settings.h"

#include <vsg/ui/Keyboard.h>

#include <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CameraWalkManipulator::CameraWalkManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                                             vsg::ref_ptr<vsg::Camera> camera,
                                             settings_t& settings,
                                             PlayerController& player,
                                             collision::CollisionWorld* world)
    : CameraAbstract(keyboard, camera, settings)
    , _player(player)
    , _world(world)
{
    _fov_current = _settings.fovy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::activate(const vsg::dvec3& spawn_pos)
{
    _player.teleport(collision::Vec3f(static_cast<float>(spawn_pos.x),
                                      static_cast<float>(spawn_pos.y),
                                      static_cast<float>(spawn_pos.z)));

    // Взгляд вдоль направления прежней камеры (горизонтальная проекция)
    vsg::dvec3 look = _lookAt->center - _lookAt->eye;

    const double len = std::sqrt(look.x * look.x + look.y * look.y);

    if (len > 1e-6)
    {
        // forward = (-sin(yaw), cos(yaw)): yaw из направления взгляда
        _yaw = std::atan2(-look.x, look.y);
    }

    _pitch = 0.0;
    _bob_phase = 0.0;
    _bob_amplitude = 0.0;
    _landing_dip = 0.0;

    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraWalkManipulator::getYaw() const
{
    return _yaw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::sitDown(const vsg::dvec3& seat_world_pos,
                                    double eye_h)
{
    _seated = true;
    _seat_pos = seat_world_pos;
    _seat_eye_h = (eye_h > 0.3) ? eye_h : 1.15;
    _bob_phase = 0.0;
    _bob_amplitude = 0.0;
    _landing_dip = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::standUp(const vsg::dvec3& stand_world_pos)
{
    _seated = false;

    _player.teleport(collision::Vec3f(
                         static_cast<float>(stand_world_pos.x),
                         static_cast<float>(stand_world_pos.y),
                         static_cast<float>(stand_world_pos.z)));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CameraWalkManipulator::isSeated() const
{
    return _seated;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::clearKeys()
{
    _own_pressed.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::keyboardPressEvent(vsg::KeySymbol key, bool pressed)
{
    if (pressed)
    {
        _own_pressed.insert(static_cast<int>(key));
    }
    else
    {
        _own_pressed.erase(static_cast<int>(key));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::resetView()
{
    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::returnView()
{
    _perspective->fieldOfViewY = _settings.fovy;
    _fov_current = _settings.fovy;
    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::mouseWheelEvent(vsg::vec3 /*delta*/)
{
    // Колесо в пешем режиме не используется (зум ломает вид от 1-го лица)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::mouseMoveEvent(vsg::ButtonMask /*button_mask*/,
                                           vsg::dvec2 delta)
{
    // Обзор мышью: события движения приходят при зажатой кнопке в
    // области рендера (см. UpdateViewerHandler::apply(vsg::MoveEvent&))
    _yaw -= delta.x * _settings.walk_mouse_sensitivity;
    _pitch += delta.y * _settings.walk_mouse_sensitivity;

    _pitch = std::max(vsg::radians(_settings.pitch_min),
               std::min(vsg::radians(_settings.pitch_max), _pitch));

    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::touchZoomEvent(double /*zoomLevel*/)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Catmull-Rom по опорным точкам (локальная реализация для анимаций)
//------------------------------------------------------------------------------
static std::vector<vsg::dvec3> walk_catmull_rom(
        const std::vector<vsg::dvec3>& cp, std::size_t segments)
{
    if (cp.size() < 2 || segments == 0)
    {
        return cp;
    }

    std::vector<vsg::dvec3> out;
    out.reserve((cp.size() - 1) * segments + 1);

    auto point_at = [&](std::size_t i)
    {
        return cp[std::min(cp.size() - 1, std::max<std::size_t>(0, i))];
    };

    for (std::size_t seg = 0; seg + 1 < cp.size(); ++seg)
    {
        const vsg::dvec3 p0 = point_at(seg - 1);
        const vsg::dvec3 p1 = point_at(seg);
        const vsg::dvec3 p2 = point_at(seg + 1);
        const vsg::dvec3 p3 = point_at(seg + 2);

        for (std::size_t j = 0; j < segments; ++j)
        {
            const double t = static_cast<double>(j) / segments;
            const double t2 = t * t;
            const double t3 = t2 * t;

            out.push_back(vsg::dvec3(
                0.5 * ((2.0 * p1.x) +
                       (-p0.x + p2.x) * t +
                       (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
                       (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3),
                0.5 * ((2.0 * p1.y) +
                       (-p0.y + p2.y) * t +
                       (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
                       (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3),
                0.5 * ((2.0 * p1.z) +
                       (-p0.z + p2.z) * t +
                       (2.0 * p0.z - 5.0 * p1.z + 4.0 * p2.z - p3.z) * t2 +
                       (-p0.z + 3.0 * p1.z - 3.0 * p2.z + p3.z) * t3)));
        }
    }

    out.push_back(cp.back());
    return out;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::startBoarding(
        const std::vector<vsg::dvec3>& path, double duration_s)
{
    // Плотная кривая Catmull-Rom по опорным точкам пути
    boarding_curve_ = walk_catmull_rom(path, 12);
    boarding_duration_ = std::max(0.5, duration_s);
    boarding_t_ = 0.0;

    boarding_z_rise_ = 0.0;
    if (!boarding_curve_.empty())
    {
        boarding_z_rise_ = std::abs(boarding_curve_.back().z -
                                    boarding_curve_.front().z);
    }

    _bob_amplitude = 0.0;
    boarding_active_ = true;
}

void CameraWalkManipulator::cancelBoarding()
{
    boarding_active_ = false;
    boarding_curve_.clear();
}

bool CameraWalkManipulator::isBoarding() const
{
    return boarding_active_;
}

double CameraWalkManipulator::playerEyeHeight() const
{
    return _player.getEyeHeight();
}



const collision::Vec3f& CameraWalkManipulator::getPlayerPosition() const
{
    return _player.getPosition();
}



void CameraWalkManipulator::shiftPlayer(const vsg::dvec3& delta)
{
    const collision::Vec3f& p = _player.getPosition();

    _player.teleport(collision::Vec3f(
                p.x + static_cast<float>(delta.x),
                p.y + static_cast<float>(delta.y),
                p.z + static_cast<float>(delta.z)));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::teleportPlayer(const vsg::dvec3& world)
{
    _player.teleport(collision::Vec3f(
                static_cast<float>(world.x),
                static_cast<float>(world.y),
                static_cast<float>(world.z)));

    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::frameEvent(double dt)
{
    //--- Посадка на сиденье (E): физика игрока не шагается, камера
    // фиксирована в точке сиденья, обзор мышью остаётся ---
    if (_seated)
    {
        _fov_current += (_settings.fovy - _fov_current) *
                std::min(1.0, _settings.walk_fov_speed * dt);

        _perspective->fieldOfViewY = _fov_current;

        updateViewSeated();
        return;
    }

    //--- Подъём по ступеням в кабину: камера по кривой, физика и
    // покачивание ходьбы отключены ---
    if (boarding_active_)
    {
        boarding_t_ = std::min(1.0, boarding_t_ + dt / boarding_duration_);

        // smoothstep для плавного старта/остановки
        const double e = boarding_t_ * boarding_t_ *
                         (3.0 - 2.0 * boarding_t_);

        const std::size_t n = boarding_curve_.size();
        const double f = e * static_cast<double>(n - 1);
        const std::size_t i0 = static_cast<std::size_t>(f);
        const std::size_t i1 = std::min(n - 1, i0 + 1);
        const double k = f - static_cast<double>(i0);

        vsg::dvec3 p = boarding_curve_[i0] * (1.0 - k) +
                       boarding_curve_[i1] * k;

        // Шаги: количество - по высоте подъёма (~28 см на ступень);
        // лёгкий вертикальный "переступ" + микро-раскачка вбок
        const int steps = std::max(2, static_cast<int>(
                std::round(boarding_z_rise_ / 0.28)));

        const double step_bob = std::sin(e * steps * 6.2831853) *
                0.030 * (1.0 - e * 0.4);
        const double step_side = std::sin(e * steps * 3.14159265) *
                0.015 * (1.0 - e * 0.4);

        const double cos_p = std::cos(_pitch);
        const double sin_p = std::sin(_pitch);

        const vsg::dvec3 dir(-std::sin(_yaw) * cos_p,
                              std::cos(_yaw) * cos_p,
                              sin_p);
        const vsg::dvec3 right(std::cos(_yaw), std::sin(_yaw), 0.0);

        _lookAt->eye = p + vsg::dvec3(0.0, 0.0, step_bob) +
                       right * step_side;
        _lookAt->center = _lookAt->eye + dir;
        _lookAt->up = vsg::dvec3(0.0, 0.0, 1.0);

        if (boarding_t_ >= 1.0)
        {
            boarding_active_ = false;
        }

        return;
    }

    // ignore_handled_keys = false: ImGui (окна статистики/настроек)
    // помечает клавиши handled, и pressed() по умолчанию их отсекает,
    // из-за чего ходьба замолкает при любом открытом окне GUI
    // Свой набор клавиш: события приходят напрямую, минуя
    // state-механику vsg::Keyboard (проверено: state врёт при
    // некоторых сценариях фокуса/повторов)
    const auto own = [this](int k)
    {
        return _own_pressed.count(k) != 0;
    };

    const bool has_shift = own(vsg::KEY_Shift_L) || own(vsg::KEY_Shift_R) ||
                           _keyboard->pressed(vsg::KEY_Shift_L, false) ||
                           _keyboard->pressed(vsg::KEY_Shift_R, false);
    const bool has_ctrl = own(vsg::KEY_Control_L) || own(vsg::KEY_Control_R) ||
                          _keyboard->pressed(vsg::KEY_Control_L, false) ||
                          _keyboard->pressed(vsg::KEY_Control_R, false);

    bool jump = own(vsg::KEY_Space) || _keyboard->pressed(vsg::KEY_Space, false);
    bool sprint = has_shift;
    bool crouch = has_ctrl;

    // WASD (и кириллические аналоги тех же клавиш - на всякий случай):
    // W=0x57/Cyr, A, S, D и KEY_Cyrillic_*
    auto has = [&](std::initializer_list<int> keys)
    {
        for (int k : keys)
        {
            if (own(k) || _keyboard->pressed(static_cast<vsg::KeySymbol>(k), false))
            {
                return true;
            }
        }
        return false;
    };

    double wish_x = 0.0;
    double wish_y = 0.0;

    // Win32-события приходят строчными KeySym ('w'=0x77), поэтому
    // принимаем оба регистра
    if (has({vsg::KEY_d, vsg::KEY_D})) wish_x += 1.0;
    if (has({vsg::KEY_a, vsg::KEY_A})) wish_x -= 1.0;
    if (has({vsg::KEY_w, vsg::KEY_W})) wish_y += 1.0;
    if (has({vsg::KEY_s, vsg::KEY_S})) wish_y -= 1.0;

    _player.step(dt, wish_x, wish_y, jump, sprint, crouch, _yaw, _world);



    const double speed = _player.getSpeed();

    //--- Покачивание головы (ТЗ п.11): частота/амплитуда зависят от
    // режима движения, при остановке плавно затухают ---
    double bob_target = 0.0;
    double bob_frequency = _settings.walk_bob_frequency;

    if (speed > 0.2 && _player.isGrounded())
    {
        if (_player.isSprinting())
        {
            bob_target = _settings.walk_bob_amplitude_run;
            bob_frequency = _settings.walk_bob_frequency_run;
        }
        else if (_player.isCrouched())
        {
            bob_target = _settings.walk_bob_amplitude * 0.5;
        }
        else
        {
            bob_target = _settings.walk_bob_amplitude;
        }
    }

    _bob_amplitude += (bob_target - _bob_amplitude) * std::min(1.0, 4.0 * dt);
    _bob_phase += speed * bob_frequency * dt;

    //--- Приземление (ТЗ п.13): проседание по скорости удара ---
    if (_player.justLanded())
    {
        _landing_dip = std::min(_player.getLandingImpact() *
                                _settings.walk_landing_coeff,
                                _settings.walk_landing_max);
    }

    _landing_dip *= std::exp(-_settings.walk_landing_recovery * dt);

    //--- FOV при беге (ТЗ п.12) ---
    const double fov_target = _settings.fovy +
            ((_player.isSprinting() && speed > 1.0)
             ? _settings.walk_fov_boost : 0.0);

    _fov_current += (fov_target - _fov_current) *
            std::min(1.0, _settings.walk_fov_speed * dt);

    if (std::abs(_perspective->fieldOfViewY - _fov_current) > 0.01)
    {
        _perspective->fieldOfViewY = _fov_current;
    }

    updateView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::updateViewSeated()
{
    _lookAt->eye = _seat_pos + vsg::dvec3(0.0, 0.0, _seat_eye_h);

    const double cos_p = std::cos(_pitch);
    const double sin_p = std::sin(_pitch);

    _lookAt->center = _lookAt->eye +
            vsg::dvec3(-std::sin(_yaw) * cos_p,
                       std::cos(_yaw) * cos_p,
                       sin_p);
    _lookAt->up = vsg::dvec3(0.0, 0.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraWalkManipulator::updateView()
{
    const collision::Vec3f& pos = _player.getPosition();

    // Вертикальное покачивание: синус фазы шагов; горизонтальный сдвиг -
    // в противофазе (полшага) вдоль правого направления взгляда
    const double bob_z = std::sin(_bob_phase) * _bob_amplitude;
    const double bob_side = std::cos(_bob_phase * 0.5) * _bob_amplitude * 0.35;

    const double eye_z = pos.z + _player.getEyeHeight() + bob_z - _landing_dip;

    const vsg::dvec3 eye(static_cast<double>(pos.x),
                         static_cast<double>(pos.y),
                         eye_z);

    // forward = (-sin(yaw), cos(yaw)); наклон - тангаж
    const double cos_p = std::cos(_pitch);
    const double sin_p = std::sin(_pitch);

    const vsg::dvec3 dir(-std::sin(_yaw) * cos_p,
                          std::cos(_yaw) * cos_p,
                          sin_p);

    const vsg::dvec3 right(std::cos(_yaw), std::sin(_yaw), 0.0);

    _lookAt->eye = eye + right * bob_side;
    _lookAt->center = _lookAt->eye + dir;
    _lookAt->up = vsg::dvec3(0.0, 0.0, 1.0);
}
