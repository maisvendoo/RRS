//------------------------------------------------------------------------------
//
//      Brake shoes system (нагрев, износ и fade тормозных колодок)
//
//------------------------------------------------------------------------------

#include    "vehicle-brake-shoes.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>
#include    <cstdint>

namespace
{

std::uint32_t mix32(std::uint32_t value)
{
    value ^= value >> 16;
    value *= 0x85ebca6bu;
    value ^= value >> 13;
    value *= 0xc2b2ae35u;
    value ^= value >> 16;
    return value;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeShoeSystem::BrakeShoeSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeShoeSystem::loadConfig(QString cfg_path, std::size_t num_axis)
{
    pads.assign(num_axis * 2, Pad());

    // Детерминированные вариации колодок (неравномерный износ, ТЗ, п.8)
    for (std::size_t i = 0; i < pads.size(); ++i)
    {
        const double u = static_cast<double>(mix32(
                             static_cast<std::uint32_t>(i) + 17u)) /
                4294967296.0;
        pads[i].variation = 0.7 + 0.6 * u;
    }

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "BrakeShoes";

    cfg.getBool(sec, "Enabled", enabled);

    QString type_str = "";
    if (cfg.getString(sec, "Type", type_str))
    {
        if (type_str == "cast_iron")
            type = ShoeType::CastIron;
        else if (type_str == "composite")
            type = ShoeType::Composite;
        else if (type_str == "low_friction")
            type = ShoeType::LowFriction;
        else if (type_str == "high_friction")
            type = ShoeType::HighFriction;
        else if (type_str == "custom")
            type = ShoeType::Custom;
    }

    // Параметры читаем с found-флагами: пресет материала применяется
    // ТОЛЬКО к отсутствующим в конфиге ключам (значения пользователя
    // имеют приоритет над пресетом)
    double value = 0.0;

    const bool has_t_normal = cfg.getDouble(sec, "TempNormal", value);
    if (has_t_normal)
        t_normal = value;

    const bool has_t_fade = cfg.getDouble(sec, "TempFade", value);
    if (has_t_fade)
        t_fade = value;

    cfg.getDouble(sec, "TempCritical", t_critical);
    cfg.getDouble(sec, "ThermalCapacity", thermal_capacity);
    cfg.getDouble(sec, "CoolingCoeff", cooling_coeff);

    const bool has_wear = cfg.getDouble(sec, "WearPerMJ", value);
    if (has_wear)
        wear_per_mj = value;

    cfg.getDouble(sec, "MinThickness", min_thickness);
    cfg.getDouble(sec, "MaxThickness", max_thickness_mm);
    cfg.getDouble(sec, "FrictionCoefficient", friction_coeff);

    // Материал корректирует базовые характеристики (ТЗ, п.11) -
    // только те, что не заданы в конфиге явно
    switch (type)
    {
    case ShoeType::CastIron:
        if (!has_t_normal)
            t_normal = 180.0;
        if (!has_t_fade)
            t_fade = 400.0;
        if (!has_wear)
            wear_per_mj = 0.8;
        break;
    case ShoeType::Composite:
        break;
    case ShoeType::LowFriction:
        if (!has_wear)
            wear_per_mj = 0.3;
        break;
    case ShoeType::HighFriction:
        if (!has_t_normal)
            t_normal = 130.0;
        if (!has_wear)
            wear_per_mj = 0.6;
        break;
    case ShoeType::Custom:
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakeShoeSystem::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeShoeSystem::step(double dt,
                           const std::vector<double>& brake_torques,
                           const std::vector<double>& wheel_omegas,
                           double velocity,
                           double air_temperature)
{
    if (!enabled)
        return;

    // Помним температуру среды: на неё сбрасываем колодки при замене
    last_ambient = air_temperature;

    const std::size_t num_axis = pads.size() / 2;

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        const double torque = (i < brake_torques.size())
                ? std::abs(brake_torques[i]) : 0.0;
        const double omega = (i < wheel_omegas.size())
                ? std::abs(wheel_omegas[i]) : 0.0;

        // Мощность трения тормоза на оси, Вт (реальная работа, ТЗ, п.2)
        const double brake_power = torque * omega;

        for (int s = 0; s < 2; ++s)
        {
            Pad& pad = pads[i * 2 + static_cast<std::size_t>(s)];

            //--- Нагрев: доля мощности в колодку, делится на две ---
            const double heat_power = 0.5 * heat_to_shoe * brake_power;

            //--- Охлаждение: конвекция, растёт со скоростью (ТЗ, п.3) ---
            const double cooling = cooling_coeff *
                    (1.0 + std::abs(velocity) / 20.0) *
                    (pad.temperature - air_temperature);

            const double dT = (heat_power - cooling) * dt / thermal_capacity;
            pad.temperature += dT;

            // Экстремальный перегрев: повреждение колодки (ТЗ, п.4)
            if (pad.temperature > t_critical + 150.0)
            {
                pad.thickness = std::max(0.0, pad.thickness - 0.001 * dt);
            }

            //--- Износ: от работы и температуры (ТЗ, п.7) ---
            if (brake_power > 0.0)
            {
                // Работа на одну колодку - та же доля, что и в нагреве
                // (мощность оси делится на две колодки)
                const double work_mj = 0.5 * brake_power * dt / 1.0e6;

                const double temp_factor = 1.0 + wear_temp_factor *
                        std::max(0.0, (pad.temperature - t_normal)) /
                        std::max(t_critical - t_normal, 1.0);

                // Толщина новой колодки (MaxThickness): износ в долях
                const double wear = wear_per_mj * work_mj * temp_factor *
                        pad.variation / max_thickness_mm;

                pad.thickness = std::max(0.0, pad.thickness - wear);
            }

            //--- Эффективность оси: fade + износ + материал ---
            double eff = fadeFactor(pad.temperature);

            // Изношенная колодка теряет эффективность (ТЗ, п.10)
            if (pad.thickness < min_thickness)
            {
                eff *= 0.5 + 0.5 * (pad.thickness / min_thickness);
            }

            // Базовый коэффициент трения материала (конфиг)
            pad.efficiency = eff * friction_coeff;
        }
    }

    // Предупреждения (однократно через журнал, на экземпляр системы)
    if (!warned_hot && getMaxTemperature() > t_fade)
    {
        warned_hot = true;
        Journal::instance()->warning(QString(
            "[BRAKE] Shoes OVERHEATED: %1 C, efficiency fading")
            .arg(getMaxTemperature(), 0, 'f', 0));
    }
    else if (warned_hot && getMaxTemperature() < t_normal)
    {
        warned_hot = false;
    }

    if (!warned_worn && hasWornOut())
    {
        warned_worn = true;
        Journal::instance()->warning("[BRAKE] Shoe(s) WORN OUT");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::fadeFactor(double temperature) const
{
    if (temperature <= t_normal)
        return 1.0;

    if (temperature <= t_fade)
    {
        // Плавное снижение до eff_at_fade
        const double k = (temperature - t_normal) /
                std::max(t_fade - t_normal, 1.0);
        return 1.0 - (1.0 - eff_at_fade) * k;
    }

    if (temperature <= t_critical)
    {
        const double k = (temperature - t_fade) /
                std::max(t_critical - t_fade, 1.0);
        return eff_at_fade - (eff_at_fade - eff_at_critical) * k;
    }

    return eff_over_critical;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::getAxleEfficiency(std::size_t axle) const
{
    if (axle * 2 + 1 >= pads.size())
        return 1.0;

    return 0.5 * (pads[axle * 2].efficiency + pads[axle * 2 + 1].efficiency);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::getAverageEfficiency() const
{
    if (pads.empty())
        return 1.0;

    double sum = 0.0;
    for (const Pad& pad : pads)
        sum += pad.efficiency;

    return sum / static_cast<double>(pads.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::getMaxTemperature() const
{
    double max_t = 0.0;
    for (const Pad& pad : pads)
        max_t = std::max(max_t, pad.temperature);

    return max_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::getTemperature(std::size_t axle, int side) const
{
    const std::size_t idx = axle * 2 +
            static_cast<std::size_t>(side >= 0 ? side : 0);

    if (idx >= pads.size())
        return 20.0;

    return pads[idx].temperature;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeShoeSystem::getThickness(std::size_t axle, int side) const
{
    const std::size_t idx = axle * 2 +
            static_cast<std::size_t>(side >= 0 ? side : 0);

    if (idx >= pads.size())
        return 1.0;

    return pads[idx].thickness;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakeShoeSystem::hasWornOut() const
{
    for (const Pad& pad : pads)
    {
        if (pad.thickness < min_thickness)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeShoeSystem::replaceShoes()
{
    for (Pad& pad : pads)
    {
        pad.thickness = 1.0;
        // Новые колодки стартуют с температуры окружающей среды
        // (последней известной из step), а не с хардкода
        pad.temperature = last_ambient;
        pad.efficiency = 1.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString BrakeShoeSystem::getDebugMsg() const
{
    QString msg = QString("Brake shoes: avg eff %1%, max T %2 C")
            .arg(getAverageEfficiency() * 100.0, 0, 'f', 0)
            .arg(getMaxTemperature(), 0, 'f', 0);

    const std::size_t num_axis = pads.size() / 2;
    for (std::size_t i = 0; i < num_axis; ++i)
    {
        msg += QString(" [%1: %2C/%3mm %4C/%5mm]")
                .arg(i)
                .arg(pads[i*2].temperature, 0, 'f', 0)
                .arg(pads[i*2].thickness * max_thickness_mm, 0, 'f', 1)
                .arg(pads[i*2+1].temperature, 0, 'f', 0)
                .arg(pads[i*2+1].thickness * max_thickness_mm, 0, 'f', 1);
    }

    return msg;
}
