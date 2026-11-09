//------------------------------------------------------------------------------
//
//      Energy meter system (расход электроэнергии и статистика рейса)
//
//------------------------------------------------------------------------------

#include    "vehicle-energy.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnergyMeterSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Energy";

    QString type_str = "";
    if (cfg.getString(sec, "DriveType", type_str))
    {
        if (type_str == "diesel")
            type = DriveType::Diesel;
        else if (type_str == "hybrid")
            type = DriveType::Hybrid;
        else
            type = DriveType::Electric;
    }

    cfg.getDouble(sec, "TractionEfficiency", traction_efficiency);
    cfg.getDouble(sec, "RegenEfficiency", regen_efficiency);
    cfg.getDouble(sec, "AuxPower", aux_power);
    cfg.getDouble(sec, "NominalVoltage", nominal_voltage);
    cfg.getDouble(sec, "CurrentLimit", current_limit);
    cfg.getDouble(sec, "MaxRegenPower", max_regen_power);
    cfg.getDouble(sec, "MinRegenSpeed", min_regen_speed);
    cfg.getDouble(sec, "ContinuousPower", continuous_power);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnergyMeterSystem::step(double dt, double traction_force, double velocity,
                             double line_voltage,
                             double network_regen_accept_w,
                             bool network_accepts_regen)
{
    const double abs_v = std::abs(velocity);

    // Механическая мощность на ободе: P = F * v (Вт)
    const double p_mech = traction_force * velocity;

    double regen_return_w = 0.0;   ///< фактический возврат в сеть, Вт

    if (p_mech >= 0.0)
    {
        // Тяга: электрическая мощность больше механической на КПД +
        // вспомогательные нужды (работают и на стоянке).
        // Перегретое тяговое оборудование ограничивает мощность,
        // ограничение по току (с прошлого шага) срезает учтённую
        // мощность тяги (сам факт ограничения см. getPowerLimitFactor)
        const double thermal_factor = 1.0 - 0.5 * traction_thermal;

        power_kw = p_mech / 1000.0 * thermal_factor * limit_factor /
                std::max(traction_efficiency, 0.1) + aux_power;
    }
    else
    {
        // Электрическое торможение (ТЗ "Рекуперация", п.2-8):
        // доступная рекуперативная мощность ограничена локомотивом
        // (max_regen_power, ток, минимальная скорость) и сетью
        // (приём подстанции/потребителей). Непринятая часть - реостат
        double available_w = std::abs(p_mech) * regen_efficiency;

        if (abs_v < min_regen_speed)
            available_w = 0.0;

        available_w = std::min(available_w, max_regen_power * 1000.0);

        // Перенапряжение: возврат нельзя, если сеть уже на пределе.
        // Порог = 1.08 номинала (AC 25 кВ -> 27 кВ; DC 3 кВ -> 3.24 кВ)
        const double overvoltage = (line_voltage > 1000.0)
                ? line_voltage / (1.08 * nominal_voltage) : 0.0;

        // Без контакта с КС (Uks ~ 0, нейтральная вставка, потеря
        // токоприёмника) рекуперация в сеть невозможна: вся
        // доступная энергия идёт в реостат
        double accept_w = (network_accepts_regen && line_voltage > 100.0)
                ? network_regen_accept_w : 0.0;

        if (overvoltage > 1.0)
            accept_w = 0.0;

        regen_return_w = std::min(available_w, accept_w);

        const double dissipated_w = available_w - regen_return_w;

        dissipated_kwh += dissipated_w * dt / 3600.0 / 1000.0;

        power_kw = -regen_return_w / 1000.0 + aux_power;
    }

    // Тепловая модель тягового оборудования: рост выше продолжительной
    // мощности, охлаждение ниже неё (ТЗ, п.9-11)
    const double overload_kw = std::max(std::abs(power_kw) - continuous_power, 0.0);

    const double heat_rate = overload_kw / std::max(continuous_power * 2.0, 1.0);
    const double cool_rate = (traction_thermal > 0.0) ? 0.01 : 0.0;

    traction_thermal = std::min(1.0, std::max(0.0,
            traction_thermal + (heat_rate * 0.02 - cool_rate) * dt));

    // Ток из КС (электрический/гибридный привод и есть КС)
    const double voltage = (line_voltage > 100.0) ? line_voltage
                                                  : nominal_voltage;

    if (type == DriveType::Diesel)
    {
        current_a = 0.0;
    }
    else
    {
        current_a = std::abs(power_kw) * 1000.0 / std::max(voltage, 100.0);
    }

    // Ограничение по току: выше предела - тяге запрещено наращивать
    // (фактор применяется к учтённой мощности тяги со следующего шага)
    if (current_a > current_limit)
    {
        limit_factor = std::max(0.2, current_limit / std::max(current_a, 1.0));
    }
    else
    {
        limit_factor = 1.0;
    }

    // Интегрирование энергии
    const double kwh = power_kw * dt / 3600.0;

    if (kwh >= 0.0)
        consumed_kwh += kwh;
    else
        regenerated_kwh += -kwh;

    peak_power_kw = std::max(peak_power_kw, power_kw);

    // Путь по скорости (для удельного расхода)
    distance_km += abs_v * dt / 1000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getPower() const
{
    return power_kw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getCurrent() const
{
    return current_a;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getPowerLimitFactor() const
{
    return limit_factor;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getLastPowerLimitFactor() const
{
    // Фактор, рассчитанный в последнем step() (он же применяется
    // к учтённой мощности тяги на следующем шаге)
    return limit_factor;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getConsumed() const
{
    return consumed_kwh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getRegenerated() const
{
    return regenerated_kwh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getDissipated() const
{
    return dissipated_kwh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getTractionThermal() const
{
    return traction_thermal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getDistance() const
{
    return distance_km;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getSpecificConsumption() const
{
    if (distance_km < 0.1)
        return 0.0;

    return (consumed_kwh - regenerated_kwh) / distance_km;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EnergyMeterSystem::getPeakPower() const
{
    return peak_power_kw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnergyMeterSystem::resetTrip()
{
    consumed_kwh = 0.0;
    regenerated_kwh = 0.0;
    dissipated_kwh = 0.0;
    distance_km = 0.0;
    peak_power_kw = 0.0;
    traction_thermal = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString EnergyMeterSystem::getDebugMsg() const
{
    return QString("Energy: %1 kW (%2 A, limit %3%), consumed %4 kWh, "
                   "regen %5 kWh, %6 kWh/km, %7 km")
            .arg(power_kw, 0, 'f', 0)
            .arg(current_a, 0, 'f', 0)
            .arg(limit_factor * 100.0, 0, 'f', 0)
            .arg(consumed_kwh, 0, 'f', 1)
            .arg(regenerated_kwh, 0, 'f', 1)
            .arg(getSpecificConsumption(), 0, 'f', 1)
            .arg(distance_km, 0, 'f', 1);
}
