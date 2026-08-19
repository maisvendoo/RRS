//------------------------------------------------------------------------------
//
//      Vehicle damage system (компонентные повреждения 0..1)
//
//------------------------------------------------------------------------------

#include    "vehicle-damage.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDamageSystem::VehicleDamageSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDamageSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "DamageSystem";

    cfg.getDouble(sec, "BodyBreakEnergy", body_break_energy);
    cfg.getDouble(sec, "BogieBreakEnergy", bogie_break_energy);
    cfg.getDouble(sec, "CouplerBreakEnergy", coupler_break_energy);
    cfg.getDouble(sec, "BrakeBreakEnergy", brake_break_energy);
    cfg.getDouble(sec, "TankBreakEnergy", tank_break_energy);
    cfg.getDouble(sec, "ImpactThreshold", impact_threshold);
    cfg.getDouble(sec, "DerailWearRate", derail_wear_rate);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDamageSystem::applyImpact(double energy, ImpactZone zone)
{
    if (energy < impact_threshold)
        return;

    // Распределение энергии удара по компонентам в зависимости от зоны.
    // Сумма долей > 1: часть энергии необратимо уходит в деформации
    switch (zone)
    {
    case ImpactZone::Front:
    case ImpactZone::Rear:
    {
        // Продольный удар: сцепки, рама, тележки; при очень сильном -
        // смещение груза повреждает кузов
        addDamage(Component::Coupler, energy / coupler_break_energy);
        addDamage(Component::Body, 0.3 * energy / body_break_energy);
        addDamage(Component::Bogie, 0.2 * energy / bogie_break_energy);
        addDamage(Component::Brake, 0.1 * energy / brake_break_energy);
        break;
    }

    case ImpactZone::Side:
    {
        // Боковой удар: кузов, тележки, колёса; опасен опрокидыванием
        addDamage(Component::Body, 0.6 * energy / body_break_energy);
        addDamage(Component::Bogie, 0.5 * energy / bogie_break_energy);
        addDamage(Component::Wheel, 0.3 * energy / bogie_break_energy);
        break;
    }

    case ImpactZone::Bottom:
    {
        // Удар снизу / движение после схода: ходовая, колёса, тормоза
        addDamage(Component::Bogie, 0.8 * energy / bogie_break_energy);
        addDamage(Component::Wheel, 0.6 * energy / bogie_break_energy);
        addDamage(Component::Brake, 0.5 * energy / brake_break_energy);
        addDamage(Component::Tank, 0.3 * energy / tank_break_energy);
        break;
    }

    case ImpactZone::Top:
    {
        // Переворот/падение: кузов, электро, силовая установка
        addDamage(Component::Body, energy / body_break_energy);
        addDamage(Component::Electrical, 0.5 * energy / body_break_energy);
        addDamage(Component::Engine, 0.5 * energy / body_break_energy);
        addDamage(Component::Tank, 0.4 * energy / tank_break_energy);
        break;
    }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDamageSystem::applyDerailmentWear(double dt, double velocity)
{
    // Движение по шпалам/балласту интенсивно разрушает ПЕ
    const double rate = derail_wear_rate * std::min(std::abs(velocity), 25.0);

    addDamage(Component::Bogie, rate * dt);
    addDamage(Component::Wheel, 1.5 * rate * dt);
    addDamage(Component::Brake, rate * dt);
    addDamage(Component::Body, 0.4 * rate * dt);
    addDamage(Component::Tank, 0.2 * rate * dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDamageSystem::addDamage(Component component, double value)
{
    const auto idx = static_cast<std::size_t>(component);

    if (idx >= static_cast<std::size_t>(Component::Count))
        return;

    damage[idx] = std::min(1.0, damage[idx] + std::max(0.0, value));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleDamageSystem::getDamage(Component component) const
{
    const auto idx = static_cast<std::size_t>(component);

    if (idx >= static_cast<std::size_t>(Component::Count))
        return 0.0;

    return damage[idx];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleDamageSystem::getBrakeEfficiency() const
{
    // Тормоза деградируют с повреждением тормозного оборудования и ходовой
    const double brake = getDamage(Component::Brake);
    const double bogie = getDamage(Component::Bogie);

    return std::max(0.0, 1.0 - 0.8 * brake - 0.2 * bogie);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleDamageSystem::getCouplerStrengthFactor() const
{
    return std::max(0.0, 1.0 - getDamage(Component::Coupler));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleDamageSystem::isDestroyed() const
{
    return (getDamage(Component::Body) >= 1.0) ||
           (getDamage(Component::Bogie) >= 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDamageSystem::reset()
{
    for (double& value : damage)
        value = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehicleDamageSystem::getDebugMsg() const
{
    return QString("Damage: coupler=%1% bogie=%2% wheel=%3% brake=%4% "
                   "body=%5% elect=%6% engine=%7% tank=%8%")
            .arg(getDamage(Component::Coupler) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Bogie) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Wheel) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Brake) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Body) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Electrical) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Engine) * 100.0, 0, 'f', 0)
            .arg(getDamage(Component::Tank) * 100.0, 0, 'f', 0);
}
