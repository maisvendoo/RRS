//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Irregularity shape functions
//
//------------------------------------------------------------------------------

#include    "track-irregularity.h"

#include    <algorithm>
#include <cmath>
#include    <cstring>

namespace track
{

namespace
{

constexpr double pi = 3.14159265358979323846;

/// Сглаживание smoothstep: 0 при t<=0, 1 при t>=1, C1-непрерывно на краях
double smoothstep(double t)
{
    t = std::min(1.0, std::max(0.0, t));
    return t * t * (3.0 - 2.0 * t);
}

/// Форма "горка" sin^2 на отрезке [x0, x0 + len]: 0 на краях, 1 в середине
double sin2Window(double x, double x0, double len)
{
    const double phase = (x - x0) / len;
    if (phase <= 0.0 || phase >= 1.0)
        return 0.0;

    const double s = std::sin(pi * phase);
    return s * s;
}

/// Форма рельсового стыка (половина волны косинуса) с центром в точке
/// center на отрезке длины len: 0 на краях, 1 в центре. Используется
/// самой неровностью RailJoint и составной формой Switch
double jointWave(double x, double center, double len)
{
    const double half = 0.5 * len;
    const double phase = (x - center + half) / len; // 0..1 внутри зоны
    if (phase <= 0.0 || phase >= 1.0)
        return 0.0;

    return 0.5 * (1.0 - std::cos(2.0 * pi * phase));
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Irregularity::offset(double path_coord) const
{
    const double x0 = beginCoord();
    const double x1 = endCoord();

    if (path_coord <= x0 || path_coord >= x1)
        return 0.0;

    switch (type)
    {
    case IrregularityType::RailJoint:
    {
        // Резкая просадка: половина волны косинуса с центром в coord.
        // Максимум глубины - в центре зоны (нуль на краях): один удар
        // на стык, а не два по краям зоны
        const double half = 0.5 * length;
        const double phase = (path_coord - coord + half) / length; // 0..1
        const double cos_wave = 0.5 * (1.0 - std::cos(2.0 * pi * phase));
        const double wear_scale = 0.5 + 0.5 * std::min(1.0, std::max(0.0, wear));
        return -amplitude * wear_scale * cos_wave;
    }

    case IrregularityType::Weld:
    {
        // Плавная горка sin^2, амплитуда падает с качеством сварки:
        // качественный шов практически не ощущается (ТЗ, п.4)
        const double quality = std::min(1.0, std::max(0.0, this->quality));
        const double amp = amplitude * (1.0 - 0.9 * quality);
        return -amp * sin2Window(path_coord, x0, length);
    }

    case IrregularityType::Depression:
    {
        // Просадка: плоское дно со сглаженными склонами (ТЗ, п.5):
        // вход [x0, x0+entry], дно, выход [x1-exit, x1]
        double depth = 0.0;

        if (path_coord < x0 + entry_length)
            depth = smoothstep((path_coord - x0) / std::max(entry_length, 1e-6));
        else if (path_coord > x1 - exit_length)
            depth = smoothstep((x1 - path_coord) / std::max(exit_length, 1e-6));
        else
            depth = 1.0;

        return -amplitude * depth;
    }

    case IrregularityType::LocalDip:
        return -amplitude * sin2Window(path_coord, x0, length);

    case IrregularityType::Bulge:
        return amplitude * sin2Window(path_coord, x0, length);

    case IrregularityType::WaveWear:
    {
        // Синус с амплитудным окном sin^2 по всей длине участка
        const double wl = std::max(wavelength, 0.05);
        const double phase = 2.0 * pi * (path_coord - x0) / wl;
        return amplitude * std::sin(phase) * sin2Window(path_coord, x0, length);
    }

    case IrregularityType::Switch:
    {
        // Стрелочный перевод (ТЗ "Неровности пути", п.15): тройной удар.
        // Вход/выход зоны - импульсы формы рельсового стыка с амплитудой
        // x1.5 (стыки перед контррельсами и за крестовиной), в центре -
        // жёсткий удар о крестовину (короткий, двойная амплитуда).
        // Износ перевода масштабирует амплитуду (паттерн стыка)
        const double entry_exit_len = 0.2 * length;
        const double frog_len = 0.15 * length;

        const double entry_center = x0 + 0.5 * entry_exit_len;
        const double exit_center = x1 - 0.5 * entry_exit_len;

        const double entry = jointWave(path_coord, entry_center, entry_exit_len);
        const double exit = jointWave(path_coord, exit_center, entry_exit_len);
        const double frog = jointWave(path_coord, coord, frog_len);

        const double wear_scale = 0.5 + 0.5 * std::min(1.0, std::max(0.0, wear));

        return -amplitude * wear_scale * (1.5 * (entry + exit) + 2.0 * frog);
    }
    }

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Irregularity::beginCoord() const
{
    return coord - 0.5 * length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Irregularity::endCoord() const
{
    return coord + 0.5 * length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IrregularityType Irregularity::typeFromString(const char* name, bool* ok)
{
    if (ok != nullptr)
        *ok = true;

    if (std::strcmp(name, "joint") == 0)
        return IrregularityType::RailJoint;
    if (std::strcmp(name, "weld") == 0)
        return IrregularityType::Weld;
    if (std::strcmp(name, "depression") == 0)
        return IrregularityType::Depression;
    if (std::strcmp(name, "dip") == 0)
        return IrregularityType::LocalDip;
    if (std::strcmp(name, "bulge") == 0)
        return IrregularityType::Bulge;
    if (std::strcmp(name, "wave") == 0)
        return IrregularityType::WaveWear;
    if (std::strcmp(name, "switch") == 0)
        return IrregularityType::Switch;

    if (ok != nullptr)
        *ok = false;
    return IrregularityType::LocalDip;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char* Irregularity::typeToString(IrregularityType type)
{
    switch (type)
    {
    case IrregularityType::RailJoint:   return "joint";
    case IrregularityType::Weld:        return "weld";
    case IrregularityType::Depression:  return "depression";
    case IrregularityType::LocalDip:    return "dip";
    case IrregularityType::Bulge:       return "bulge";
    case IrregularityType::WaveWear:    return "wave";
    case IrregularityType::Switch:      return "switch";
    }

    return "dip";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RailSide Irregularity::sideFromString(const char* name, bool* ok)
{
    if (ok != nullptr)
        *ok = true;

    if (std::strcmp(name, "left") == 0)
        return RailSide::Left;
    if (std::strcmp(name, "right") == 0)
        return RailSide::Right;
    if (std::strcmp(name, "both") == 0)
        return RailSide::Both;

    if (ok != nullptr)
        *ok = false;
    return RailSide::Both;
}

} // namespace track
