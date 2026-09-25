#include "CabSway.h"

#include <cmath>

namespace
{

constexpr double SWAY_2PI = 6.28318530717958647692;

/// Синус компонента по пройденному пути
double swaySin(double path_s, double wavelength, double phase)
{
    return std::sin(SWAY_2PI * path_s / wavelength + phase);
}

/// Детерминированный хэш адреса ПЕ в фазовый сдвиг, рад
double hashPhase(const void* vehicle, unsigned salt)
{
    if (vehicle == nullptr)
        return 0.0;

    auto value = reinterpret_cast<uintptr_t>(vehicle);
    value ^= salt * 0x9E3779B97F4A7C15ull;
    value *= 0xBF58476D1CE4E5B9ull;
    value ^= value >> 29;

    return SWAY_2PI * static_cast<double>(value % 1000u) / 1000.0;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabSway::setParams(const Params& params)
{
    _params = params;
    _params.ref_speed = std::max(_params.ref_speed, 1.0);
    _params.min_speed = std::min(std::max(_params.min_speed, 0.0),
                                 0.5 * _params.ref_speed);
    _params.max_offset = std::max(_params.max_offset, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabSway::setVehicle(const void* vehicle)
{
    _phase_bounce = hashPhase(vehicle, 1);
    _phase_gallop = hashPhase(vehicle, 2);
    _phase_hunt = hashPhase(vehicle, 3);
    _phase_roll = hashPhase(vehicle, 4);
    _phase_pitch = hashPhase(vehicle, 5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabSway::step(double dt, double speed)
{
    _offset_x = 0.0;
    _offset_y = 0.0;
    _offset_z = 0.0;
    _roll = 0.0;
    _pitch = 0.0;

    if (!_params.enabled || dt <= 0.0)
        return;

    // Фаза раскачки - пройденный путь: частота автоматически
    // пропорциональна скорости, разрывов при смене направления нет
    _path_s += std::abs(speed) * dt;

    // Модуляция по скорости: покой ниже min_speed, полная амплитуда
    // от ref_speed, раскрутка квадратичная - трогание не трясёт
    const double v = std::abs(speed);
    double q = 0.0;

    if (v >= _params.ref_speed)
    {
        q = 1.0;
    }
    else if (v > _params.min_speed)
    {
        q = (v - _params.min_speed) / (_params.ref_speed - _params.min_speed);
        q = q * q;
    }

    if (q <= 0.0)
        return;

    // Кузов на рессорах: два вертикальных тона (подпрыгивание +
    // галопирование), виляние поперёк, покачивание и клевки
    _offset_z = q * (_params.bounce_amp * swaySin(_path_s, BOUNCE_WAVELENGTH, _phase_bounce) +
                     _params.gallop_amp * swaySin(_path_s, GALLOP_WAVELENGTH, _phase_gallop));
    _offset_x = q * _params.hunt_amp * swaySin(_path_s, HUNT_WAVELENGTH, _phase_hunt);
    _roll = q * _params.roll_amp * swaySin(_path_s, ROLL_WAVELENGTH, _phase_roll);
    _pitch = q * _params.pitch_amp * swaySin(_path_s, PITCH_WAVELENGTH, _phase_pitch);

    _offset_z = std::min(std::max(_offset_z, -_params.max_offset), _params.max_offset);
    _offset_x = std::min(std::max(_offset_x, -_params.max_offset), _params.max_offset);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::dvec3 CabSway::offset() const
{
    return vsg::dvec3(_offset_x, _offset_y, _offset_z);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CabSway::roll() const
{
    return _roll;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CabSway::pitch() const
{
    return _pitch;
}
