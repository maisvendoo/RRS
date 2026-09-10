//------------------------------------------------------------------------------
//
//      Hazard cargo system (опасные грузы, утечки, пожар, взрыв)
//
//------------------------------------------------------------------------------

#include    "vehicle-hazard.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>
#include    <cstdint>

namespace
{

const char* cargoTypeName(VehicleHazard::CargoType type)
{
    switch (type)
    {
    case VehicleHazard::CargoType::Normal:      return "normal";
    case VehicleHazard::CargoType::Flammable:   return "flammable";
    case VehicleHazard::CargoType::Explosive:   return "explosive";
    case VehicleHazard::CargoType::Toxic:       return "toxic";
    case VehicleHazard::CargoType::Corrosive:   return "corrosive";
    case VehicleHazard::CargoType::Pressurized: return "pressurized";
    }

    return "normal";
}

/// Детерминированный источник псевдослучайности для игровых событий
/// (воспламенение): без глобального состояния rand()
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleHazard::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Cargo";

    QString type_str = "";
    if (cfg.getString(sec, "Type", type_str))
    {
        if (type_str == "flammable")
            cargo = CargoType::Flammable;
        else if (type_str == "explosive")
            cargo = CargoType::Explosive;
        else if (type_str == "toxic")
            cargo = CargoType::Toxic;
        else if (type_str == "corrosive")
            cargo = CargoType::Corrosive;
        else if (type_str == "pressurized")
            cargo = CargoType::Pressurized;
        else
            cargo = CargoType::Normal;
    }

    cfg.getDouble(sec, "LeakDamageThreshold", leak_damage_threshold);
    cfg.getDouble(sec, "IgniteRate", ignite_rate);
    cfg.getDouble(sec, "FireGrowth", fire_growth);
    cfg.getDouble(sec, "ExplosionThreshold", explosion_threshold);
    cfg.getDouble(sec, "FireBurnoutTime", fire_burnout_time);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleHazard::step(double dt,
                         double tank_damage,
                         double body_damage,
                         bool fire_nearby)
{
    explosion_event = false;

    // Прогрессия состояния ёмкости (ТЗ, п.23): Normal -> Damaged -> Leak
    if (cargo == CargoType::Normal)
        return;

    const double damage_level = std::max(tank_damage,
                                         0.5 * body_damage);

    if (damage_level >= 1.0)
    {
        tank_state = TankState::Critical;
    }
    else if (damage_level >= leak_damage_threshold)
    {
        if (tank_state == TankState::Normal)
        {
            tank_state = TankState::Damaged;
        }
        else if (tank_state == TankState::Damaged)
        {
            tank_state = TankState::Leak;

            Journal::instance()->warning(QString(
                "[HAZARD] Tank LEAK detected (%1 cargo)")
                .arg(cargoTypeName(cargo)));
        }
    }

    // Возгорание: утечка горючего/газа + источник (внешний пожар или
    // самовоспламенение с интенсивностью ignite_rate)
    if (!on_fire && !exploded)
    {
        if (tank_state >= TankState::Leak)
        {
            if (cargo == CargoType::Flammable ||
                cargo == CargoType::Pressurized ||
                cargo == CargoType::Explosive)
            {
                // Детерминированная попытка воспламенения: хэш времени
                // утечки против вероятности за шаг
                const auto tick = static_cast<std::uint32_t>(fire_time * 1000.0);
                const double u = static_cast<double>(mix32(tick + 1u)) /
                        4294967296.0;

                tryIgnite(fire_nearby || u < ignite_rate * dt);
            }
        }
    }

    // Развитие/затухание пожара
    if (on_fire)
    {
        fire_time += dt;

        if (fire_time > fire_burnout_time)
        {
            // Выгорание
            fire_intensity -= 0.02 * dt;

            if (fire_intensity <= 0.0)
            {
                on_fire = false;
                fire_intensity = 0.0;
            }
        }
        else
        {
            fire_intensity = std::min(1.0, fire_intensity + fire_growth * dt);
        }

        // Взрыв: критическое состояние ёмкости при пожаре (не случайный
        // эффект - следствие состояния, ТЗ, п.25)
        if (!exploded &&
            (cargo == CargoType::Explosive || cargo == CargoType::Pressurized) &&
            tank_state >= TankState::Critical &&
            fire_intensity > explosion_threshold)
        {
            exploded = true;
            explosion_event = true;
            on_fire = false;
            fire_intensity = 0.0;

            Journal::instance()->critical(QString(
                "[HAZARD] EXPLOSION of %1 cargo!")
                .arg(cargoTypeName(cargo)));
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleHazard::tryIgnite(bool condition)
{
    if (condition)
    {
        on_fire = true;
        fire_intensity = 0.1;
        fire_time = 0.0;

        Journal::instance()->warning(QString(
            "[HAZARD] FIRE started (%1 cargo)")
            .arg(cargoTypeName(cargo)));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleHazard::CargoType VehicleHazard::getCargoType() const
{
    return cargo;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleHazard::TankState VehicleHazard::getTankState() const
{
    return tank_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleHazard::isOnFire() const
{
    return on_fire;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleHazard::getFireIntensity() const
{
    return fire_intensity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleHazard::consumeExplosionEvent()
{
    const bool event = explosion_event;
    explosion_event = false;
    return event;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleHazard::getHazardRadius() const
{
    // Радиус воздействия: пожар ~30 м, взрыв (однократно) до 80 м,
    // токсичная утечка ~50 м
    if (on_fire)
        return 30.0;

    if (exploded)
        return 80.0;

    if (tank_state >= TankState::Leak && cargo == CargoType::Toxic)
        return 50.0;

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleHazard::isZoneDangerous() const
{
    return on_fire || tank_state >= TankState::Leak;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleHazard::reset()
{
    tank_state = TankState::Normal;
    on_fire = false;
    fire_intensity = 0.0;
    explosion_event = false;
    exploded = false;
    fire_time = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehicleHazard::getDebugMsg() const
{
    const char* states[] = {"normal", "damaged", "leak", "critical"};

    return QString("Hazard: cargo=%1 tank=%2 fire=%3")
            .arg(cargoTypeName(cargo))
            .arg(states[static_cast<int>(tank_state)])
            .arg(on_fire ? QString::number(fire_intensity, 'f', 2) : QString("no"));
}
