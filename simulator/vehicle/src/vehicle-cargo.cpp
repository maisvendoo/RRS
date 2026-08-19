//------------------------------------------------------------------------------
//
//      Cargo system (погрузка/разгрузка грузовых вагонов)
//
//------------------------------------------------------------------------------

#include    "vehicle-cargo.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CargoSystem::CargoSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CargoSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "CargoWagon";

    bool found = false;
    found = cfg.getDouble(sec, "MaxLoad", max_load_kg) || found;
    found = cfg.getDouble(sec, "CurrentLoad", cargo_mass) || found;
    found = cfg.getString(sec, "Type", cargo_type) || found;

    double value = 0.0;
    if (cfg.getDouble(sec, "COMLongitudinal", value))
        com_longitudinal = value;
    if (cfg.getDouble(sec, "COMLateral", value))
        com_lateral = value;

    QString allowed = "";
    if (cfg.getString(sec, "AllowedCargo", allowed) && !allowed.isEmpty())
    {
        allowed_cargo = allowed.split(',', Qt::SkipEmptyParts);
    }

    cargo_mass = std::min(std::max(cargo_mass, 0.0), max_load_kg);

    if (cargo_mass >= max_load_kg - 1.0)
        state = State::Loaded;
    else if (cargo_mass > 0.0)
        state = State::PartiallyLoaded;

    configured = found;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CargoSystem::isConfigured() const
{
    return configured;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CargoSystem::startLoading(const QString& type)
{
    if (state == State::Loading)
        return true;

    if (state == State::Loaded)
    {
        last_error = "Вагон уже загружен";
        return false;
    }

    // Совместимость груза с вагоном (ТЗ, п.5)
    if (!allowed_cargo.isEmpty() && !allowed_cargo.contains(type))
    {
        last_error = QString("Груз %1 не совместим с вагоном").arg(type);
        Journal::instance()->warning("[CARGO] " + last_error);
        return false;
    }

    // Продолжение прерванной погрузки тем же грузом
    if (cargo_mass > 0.0 && cargo_type != type && !cargo_type.isEmpty())
    {
        last_error = QString("В вагоне уже %1").arg(cargo_type);
        return false;
    }

    cargo_type = type;
    state = State::Loading;
    last_error = "";

    Journal::instance()->info(QString(
        "[CARGO] Loading '%1' started (%2 t loaded)")
        .arg(cargo_type)
        .arg(cargo_mass / 1000.0, 0, 'f', 1));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CargoSystem::startUnloading()
{
    if (cargo_mass <= 0.0)
    {
        last_error = "Вагон пуст";
        return false;
    }

    state = State::Unloading;
    last_error = "";

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CargoSystem::stop()
{
    if (state == State::Loading || state == State::Unloading)
    {
        // Прерванная операция: груз остаётся (п.15)
        state = (cargo_mass >= max_load_kg - 1.0) ? State::Loaded
                : (cargo_mass > 0.0 ? State::PartiallyLoaded : State::Empty);

        Journal::instance()->info(QString(
            "[CARGO] Operation stopped at %1 t")
            .arg(cargo_mass / 1000.0, 0, 'f', 1));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CargoSystem::step(double dt, double rate_t_per_hour)
{
    if (state == State::Loading)
    {
        // Постепенная погрузка (ТЗ, п.9): кг/с от оборудования
        const double rate_kg_s = rate_t_per_hour * 1000.0 / 3600.0;

        cargo_mass = std::min(max_load_kg, cargo_mass + rate_kg_s * dt);

        if (cargo_mass >= max_load_kg)
        {
            state = State::Loaded;

            Journal::instance()->info(QString(
                "[CARGO] '%1' LOADED: %2 t")
                .arg(cargo_type)
                .arg(cargo_mass / 1000.0, 0, 'f', 1));
        }
    }
    else if (state == State::Unloading)
    {
        const double rate_kg_s = rate_t_per_hour * 1000.0 / 3600.0;

        cargo_mass = std::max(0.0, cargo_mass - rate_kg_s * dt);

        if (cargo_mass <= 0.0)
        {
            state = State::Empty;
            cargo_type = "";

            Journal::instance()->info("[CARGO] Wagon emptied");
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CargoSystem::State CargoSystem::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CargoSystem::getCargoType() const
{
    return cargo_type;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CargoSystem::getCargoMass() const
{
    return cargo_mass;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CargoSystem::getFillLevel() const
{
    if (max_load_kg <= 0.0)
        return 0.0;

    return cargo_mass / max_load_kg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CargoSystem::getLongitudinalShift() const
{
    // Смещение ЦМ растёт с заполнением (пустой - нет смещения)
    return com_longitudinal * getFillLevel();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CargoSystem::getLateralShift() const
{
    return com_lateral * getFillLevel();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CargoSystem::getLastError() const
{
    return last_error;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CargoSystem::reset()
{
    state = State::Empty;
    cargo_type = "";
    cargo_mass = 0.0;
    last_error = "";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CargoSystem::markDelivered(const QString& delivered_cargo, double tonnes)
{
    if (tonnes <= 0.0)
        return;

    ++delivered_cargo_count;
    delivered_tonnes += tonnes;

    Journal::instance()->info(QString(
        "[CARGO] Delivered: '%1' %2 t (total: %3 / %4 t)")
        .arg(delivered_cargo)
        .arg(tonnes, 0, 'f', 1)
        .arg(delivered_cargo_count)
        .arg(delivered_tonnes, 0, 'f', 1));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
unsigned CargoSystem::getDeliveredCargoCount() const
{
    return delivered_cargo_count;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CargoSystem::getDeliveredTonnes() const
{
    return delivered_tonnes;
}
