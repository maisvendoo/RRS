//------------------------------------------------------------------------------
//
//      Sand system (пескоподача локомотива)
//
//------------------------------------------------------------------------------

#include    "vehicle-sand.h"

#include    "vehicle-adhesion.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::loadConfig(QString cfg_path, std::size_t num_axis_)
{
    num_axis = std::max<std::size_t>(num_axis_, 2);
    nozzle_clog.assign(num_axis, 0.0);

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Sand";

    bool has_enabled = cfg.getBool(sec, "Enabled", enabled);
    bool any_key = has_enabled;
    any_key = cfg.getBool(sec, "AutoMode", auto_mode) || any_key;
    any_key = cfg.getDouble(sec, "Capacity", capacity) || any_key;
    cfg.getDouble(sec, "Amount", amount);
    cfg.getDouble(sec, "Moisture", moisture);
    cfg.getDouble(sec, "BaseRate", base_rate);
    cfg.getDouble(sec, "WetFlowLimit", wet_flow_limit);
    cfg.getDouble(sec, "SlipThreshold", slip_threshold);
    cfg.getDouble(sec, "ActivationDelay", activation_delay);
    cfg.getDouble(sec, "ReleaseDelay", release_delay);
    cfg.getDouble(sec, "SandBoost", sand_boost);

    // Секция [Sand] есть: у ПЕ есть песочница, если не указано иное
    if (any_key)
    {
        if (!has_enabled)
            enabled = true;
        if (capacity <= 0.0)
            capacity = 1000.0;
    }

    amount = std::min(amount, capacity);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::setSanding(bool on)
{
    manual_on = on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::setAutoSanding(bool on)
{
    auto_mode = on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::step(double dt,
                      double velocity,
                      bool slip,
                      WheelRailAdhesion& adhesion,
                      double rail_wet,
                      double rail_ice,
                      double rail_contam,
                      double humidity,
                      double temperature)
{
    if (!enabled)
    {
        feeding = false;
        return;
    }

    //--- Состояние песка: влажность от погоды (дождь/снег/влажность) ---

    last_temperature = temperature;

    const double wetting = 0.0002 * humidity * dt;

    // Дождь/мокрый рельс ускоряют увлажнение бункера
    moisture += wetting * (1.0 + 5.0 * rail_wet);

    // Сушка сухим тёплым воздухом
    if (temperature > 15.0 && humidity < 0.4)
    {
        moisture -= 0.00005 * dt * (temperature - 15.0);
    }

    moisture = std::min(std::max(moisture, 0.0), 1.0);

    const SandState state = getSandState();

    //--- Автоподача с гистерезисом (ТЗ, п.4) ---

    if (auto_mode)
    {
        if (slip)
        {
            auto_delay += dt;
            if (auto_delay >= activation_delay)
                auto_active = true;
        }
        else
        {
            auto_delay -= dt;
            if (auto_delay <= -release_delay)
            {
                auto_active = false;
                auto_delay = 0.0;
            }
        }
    }
    else
    {
        auto_active = false;
        auto_delay = 0.0;
    }

    //--- Фактическая подача ---

    const bool command = manual_on || auto_active;

    feeding = command && (amount > 0.1);

    // Влажный/замёрзший песок течёт хуже, форсунки засоряются
    double flow_factor = 1.0;

    switch (state)
    {
    case SandState::Dry:
        break;
    case SandState::Damp:
        flow_factor *= 0.85;
        break;
    case SandState::Wet:
        flow_factor *= 0.4;
        break;
    case SandState::Frozen:
        flow_factor *= 0.05;
        break;
    }

    if (flow_factor < 1.0 && command)
    {
        // Влажный песок забивает форсунки
        for (double& clog : nozzle_clog)
        {
            clog = std::min(1.0, clog + 0.002 * (1.0 - flow_factor) * dt);
        }
    }

    // Среднее засорение снижает расход
    double avg_clog = 0.0;
    for (double clog : nozzle_clog)
        avg_clog += clog;
    avg_clog /= static_cast<double>(nozzle_clog.size());

    flow_factor *= (1.0 - 0.8 * avg_clog);

    if (feeding)
    {
        // Расход (ТЗ, п.2): от режима и состояния
        const double rate = base_rate * flow_factor;

        amount = std::max(0.0, amount - rate * dt);

        if (amount <= 0.1)
        {
            feeding = false;
            Journal::instance()->warning("Sand box is EMPTY");
        }

        //--- Эффект на сцепление (ТЗ, п.5-6): зависит от рельса ---
        double efficiency = 1.0;

        // Мокрый рельс: песок работает хуже; лёд - сильно хуже
        efficiency -= 0.25 * rail_wet;
        efficiency -= 0.50 * rail_ice;
        efficiency -= 0.30 * rail_contam;
        efficiency *= flow_factor;

        const double boost = std::max(0.0, sand_boost * efficiency);

        // Песок под крайние оси (по направлению движения; полная схема
        // форсунок - конфигурация конкретного локомотива)
        adhesion.applySand(0, boost, 1.5);
        adhesion.applySand(num_axis - 1, boost, 1.5);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SandSystem::isFeeding() const
{
    return feeding;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SandSystem::isEnabled() const
{
    return enabled && capacity > 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandSystem::getAmount() const
{
    return amount;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandSystem::getCapacity() const
{
    return capacity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandSystem::getMoisture() const
{
    return moisture;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SandSystem::SandState SandSystem::getSandState() const
{
    if (moisture < 0.10)
        return SandState::Dry;
    if (moisture < 0.25)
        return SandState::Damp;
    if (last_temperature < -5.0 && moisture > 0.2)
        return SandState::Frozen;

    return SandState::Wet;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandSystem::getNozzleClogging() const
{
    if (nozzle_clog.empty())
        return 0.0;

    double sum = 0.0;
    for (double clog : nozzle_clog)
        sum += clog;

    return sum / static_cast<double>(nozzle_clog.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::refill(double added)
{
    amount = std::min(capacity, amount + std::max(0.0, added));
    moisture = std::min(moisture, 0.05);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandSystem::service()
{
    std::fill(nozzle_clog.begin(), nozzle_clog.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString SandSystem::getDebugMsg() const
{
    const char* states[] = {"dry", "damp", "wet", "frozen"};

    return QString("Sand: %1 kg / %2 (%3, moisture %4%), "
                   "feed %5, clog %6%")
            .arg(amount, 0, 'f', 0)
            .arg(capacity, 0, 'f', 0)
            .arg(states[static_cast<int>(getSandState())])
            .arg(moisture * 100.0, 0, 'f', 0)
            .arg(feeding ? "ON" : "off")
            .arg(getNozzleClogging() * 100.0, 0, 'f', 0);
}
