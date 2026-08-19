//------------------------------------------------------------------------------
//
//      Pantograph system (токоприёмник)
//
//------------------------------------------------------------------------------

#include    "vehicle-pantograph.h"

#include    <CfgReader.h>

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
PantographSystem::PantographSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PantographSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Pantograph";

    cfg.getDouble(sec, "StaticForce", static_force);
    cfg.getDouble(sec, "AeroCoeff", aero_coeff);
    cfg.getDouble(sec, "WireReaction", wire_reaction);
    cfg.getDouble(sec, "MinContactForce", min_contact_force);
    cfg.getDouble(sec, "WindSensitivity", wind_sensitivity);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PantographSystem::setRaised(bool value)
{
    raised = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PantographSystem::isRaised() const
{
    return raised;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PantographSystem::step(double dt, double speed, double wind_speed,
                            double wire_voltage)
{
    if (!raised)
    {
        contact_ok = false;
        contact_force = 0.0;
        arc_rate = 0.0;
        return;
    }

    const double abs_v = std::abs(speed);

    // Прижимная сила: статика + аэродинамический подъём (растёт со
    // скоростью). Реакция провода ограничивает прижим
    const double uplift = static_force + aero_coeff * abs_v * abs_v;
    contact_force = std::min(uplift, static_force + 2.0 * wire_reaction);

    // Ветер и высокие скорости раскачивают провод: с некоторой
    // вероятностью за шаг контакт кратковременно теряется (дуга)
    const double disturb = wind_sensitivity * std::abs(wind_speed) +
            aero_coeff * abs_v * abs_v / 1000.0;

    // Детерминированная "турбулентность": порог силы против возмущения
    const bool stable = (contact_force > min_contact_force) &&
            (disturb < 0.35);

    // Даже при неустойчивости контакт теряется не навсегда: часть
    // времени контакт есть (кресло проводов качается)
    bool contact = stable;

    if (!stable && wire_voltage > 100.0)
    {
        // Стохастика через хэш времени (воспроизводимо на шаге)
        const auto tick = static_cast<std::uint32_t>(arc_timer * 1000.0);
        const double u = static_cast<double>(mix32(tick + 1u)) / 4294967296.0;
        contact = u > 0.5;
    }

    if (contact != contact_ok)
    {
        // Переход контакта: потеря = дуга (если на проводе есть напряжение)
        if (!contact && wire_voltage > 100.0)
        {
            ++arcs_total;
            arc_window += dt;
        }
    }

    contact_ok = contact;

    // Частота дуг за скользящее окно 1 с
    arc_timer += dt;

    if (arc_timer >= 1.0)
    {
        arc_rate = arc_window / arc_timer;
        arc_timer = 0.0;
        arc_window = 0.0;
    }

    // Износ вставки: от силы прижима и пробега
    wear = std::min(1.0, wear + contact_force * abs_v * dt / 1.3e10);

    if (wear >= 1.0)
    {
        // Изношенный полоз хуже держит контакт
        contact_ok = contact_ok && (contact_force > 2.0 * min_contact_force);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PantographSystem::isContactOk() const
{
    return contact_ok;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PantographSystem::getContactForce() const
{
    return contact_force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PantographSystem::getArcRate() const
{
    return arc_rate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PantographSystem::getWear() const
{
    return wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString PantographSystem::getDebugMsg() const
{
    return QString("Pantograph: %1, contact %2, F=%3 N, arcs %4 (%5/s), wear %6%")
            .arg(raised ? "raised" : "lowered")
            .arg(contact_ok ? "OK" : "LOST")
            .arg(contact_force, 0, 'f', 0)
            .arg(arcs_total)
            .arg(arc_rate, 0, 'f', 1)
            .arg(wear * 100.0, 0, 'f', 1);
}
