//------------------------------------------------------------------------------
//
//      Condensate system (влага и лёд в пневматической системе)
//
//------------------------------------------------------------------------------

#include    "vehicle-condensate.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CondensateSystem::CondensateSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CondensateSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "PneumoCondensate";

    cfg.getBool(sec, "Enabled", enabled);
    cfg.getDouble(sec, "MoistureRate", moisture_rate);
    cfg.getDouble(sec, "FreezeThreshold", freeze_threshold);
    cfg.getDouble(sec, "DrainRate", drain_rate);
    cfg.getBool(sec, "AutoDrain", auto_drain);
    cfg.getDouble(sec, "FreezeRate", freeze_rate);
    cfg.getDouble(sec, "ThawRate", thaw_rate);
    cfg.getDouble(sec, "ReservoirTimeConstant", reservoir_tau);
    cfg.getDouble(sec, "CompressorHeating", compressor_heating);
    cfg.getDouble(sec, "CondensationRate", condensation_rate);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::dewPointMagnus(double T, double phi)
{
    // Формула Магнуса над водой: константы a=17.62, b=243.12 град. C.
    // Влажность ограничиваем, чтобы логарифм существовал
    const double rel = std::min(std::max(phi, 0.01), 1.0);

    const double a = 17.62;
    const double b = 243.12;

    const double gamma = std::log(rel) + a * T / (b + T);

    return b * gamma / (a - gamma);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CondensateSystem::step(double dt,
                            double air_temperature,
                            double relative_humidity,
                            bool air_charging,
                            bool standing)
{
    air_temp = std::min(std::max(air_temperature, -60.0), 60.0);
    humidity = std::min(std::max(relative_humidity, 0.0), 1.0);

    if (!enabled || dt <= 0.0)
    {
        dew_point = dewPointMagnus(air_temp, humidity);
        return;
    }

    dew_point = dewPointMagnus(air_temp, humidity);

    //--- Температура резервуаров: инерция + нагрев от компрессора (ТЗ, п.4)

    const double target = air_temp +
            (air_charging ? compressor_heating : 0.0);

    const double tau = std::max(reservoir_tau, 1.0);
    reservoir_t += (target - reservoir_t) * std::min(dt / tau, 1.0);

    //--- Влага в воздухе системы (ТЗ, п.8): растёт при зарядке магистрали
    // (компрессор засасывает влажный атмосферный воздух), медленно
    // осушается продувками/осушителем, когда зарядки нет

    if (air_charging)
        moisture += moisture_rate * (0.2 + 0.8 * humidity) * dt;
    else
        moisture -= 0.0002 * dt;

    //--- Конденсация (ТЗ, п.3, 7): холодный резервуар ниже точки росы
    // осажает воду из воздуха системы

    const double sub_dew = dew_point - reservoir_t;

    if (sub_dew > 0.0 && moisture > 0.0)
    {
        const double condensed = std::min(moisture,
                condensation_rate * sub_dew * dt);

        moisture -= condensed;
        liquid = std::min(1.0, liquid + condensed);
    }
    else if (sub_dew < -2.0 && liquid > 0.0)
    {
        // Тёплый сухой резервуар: медленное испарение обратно в воздух
        const double evaporated = std::min(liquid, 0.0001 * dt);
        liquid -= evaporated;
        moisture = std::min(1.0, moisture + evaporated);
    }

    moisture = std::min(std::max(moisture, 0.0), 1.0);

    //--- Замерзание/оттаивание (ТЗ, п.9, 14): не мгновенно, по скорости

    if (reservoir_t < freeze_threshold)
    {
        frozen_fraction = std::min(1.0,
                frozen_fraction + freeze_rate * dt);
    }
    else
    {
        // Оттаивание быстрее при большем перегреве над нулём
        const double warm = std::min(
                    (reservoir_t - freeze_threshold) / 10.0, 3.0);

        frozen_fraction = std::max(0.0,
                frozen_fraction - thaw_rate * std::max(warm, 0.1) * dt);
    }

    //--- Слив конденсата (ТЗ, п.15): открытый кран или авто-продувка
    // на стоянке (медленнее ручного слива)

    const bool draining = drain_open || (auto_drain && standing);

    if (draining && liquid > 0.0)
    {
        const double rate = drain_open ? drain_rate : 0.2 * drain_rate;
        liquid = std::max(0.0, liquid - rate * dt);
    }

    // Лёд краном не сливается: сначала оттаять (в тёплом депо)
    // Предупреждение об обледенении (ТЗ, п.20)
    if (isFrozen() && !freeze_warned)
    {
        freeze_warned = true;

        Journal::instance()->warning(QString(
            "[PNEUMO] Ice in brake system: %1% frozen water, "
            "brake response degraded to %2")
            .arg(getFrozenFraction() * 100.0, 0, 'f', 0)
            .arg(getBrakeResponseFactor(), 0, 'f', 2));
    }
    else if (!isFrozen())
    {
        freeze_warned = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getDewPoint() const
{
    return dew_point;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getAirTemperature() const
{
    return air_temp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getReservoirTemperature() const
{
    return reservoir_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getMoisture() const
{
    return moisture;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getLiquidWater() const
{
    return liquid;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getFrozenFraction() const
{
    return frozen_fraction;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getIceAmount() const
{
    return liquid * frozen_fraction;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CondensateSystem::isFrozen() const
{
    return getIceAmount() > 0.3;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CondensateSystem::getBrakeResponseFactor() const
{
    // Лёд в рукавах/кранах/воздухораспределителях сужает сечение:
    // скорость тормозной волны падает до половины (ТЗ, п.10-13)
    const double ice = getIceAmount();

    return 1.0 - 0.5 * std::min(std::max(ice, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CondensateSystem::drainValve()
{
    drain_open = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CondensateSystem::closeDrainValve()
{
    drain_open = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CondensateSystem::isDrainValveOpen() const
{
    return drain_open;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CondensateSystem::getDebugMsg() const
{
    return QString("Pneumo: T_air %1C Td %2C T_res %3C, moisture %4%, "
                   "water %5%, ice %6% (brake x%7), drain %8")
            .arg(air_temp, 0, 'f', 1)
            .arg(dew_point, 0, 'f', 1)
            .arg(reservoir_t, 0, 'f', 1)
            .arg(moisture * 100.0, 0, 'f', 0)
            .arg(liquid * 100.0, 0, 'f', 0)
            .arg(getIceAmount() * 100.0, 0, 'f', 0)
            .arg(getBrakeResponseFactor(), 0, 'f', 2)
            .arg(drain_open ? "open" : "closed");
}
