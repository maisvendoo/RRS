//------------------------------------------------------------------------------
//
//      Wheel-rail adhesion system (сцепление колёс с рельсами и погода)
//
//------------------------------------------------------------------------------

#include    "vehicle-adhesion.h"

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Adhesion";

    cfg.getDouble(sec, "SelfCleanRate", self_clean_rate);
    cfg.getDouble(sec, "DryRate", dry_rate);
    cfg.getDouble(sec, "Contamination", contamination);

    QString rail_str = "";
    if (cfg.getString(sec, "RailCondition", rail_str))
    {
        if (rail_str == "new")
            rail_condition = RailCondition::New;
        else if (rail_str == "wavy")
            rail_condition = RailCondition::WavyWear;
        else if (rail_str == "contaminated")
            rail_condition = RailCondition::Contaminated;
        else if (rail_str == "rusty")
            rail_condition = RailCondition::Rusty;
        else if (rail_str == "ground")
            rail_condition = RailCondition::Ground;
        else
            rail_condition = RailCondition::Normal;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::setWeather(Weather weather_, double intensity)
{
    weather = weather_;
    weather_intensity = std::min(std::max(intensity, 0.0), 1.0);

    // Типовая влажность воздуха для режима погоды
    switch (weather)
    {
    case Weather::Dry:          humidity = 0.3; break;
    case Weather::ExtremeHeat:  humidity = 0.2; break;
    case Weather::Humid:        humidity = 0.9; break;
    case Weather::Fog:          humidity = 0.95; break;
    case Weather::Drizzle:      humidity = 0.9; break;
    case Weather::Rain:         humidity = 0.95; break;
    case Weather::HeavyRain:    humidity = 1.0; break;
    case Weather::Snow:         humidity = 0.8; break;
    case Weather::WetSnow:      humidity = 0.9; break;
    case Weather::FreezingRain: humidity = 0.95; break;
    case Weather::Hoarfrost:    humidity = 0.85; break;
    case Weather::Ice:          humidity = 0.7; break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::setTemperature(double temperature_)
{
    temperature = temperature_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::step(double dt, double velocity, std::size_t num_axis)
{
    if (num_axis == 0)
        return;

    if (sand_boost.size() != num_axis)
    {
        sand_boost.assign(num_axis, 0.0);
        sand_timer.assign(num_axis, 0.0);
        axle_variation.assign(num_axis, 0.0);
        axle_factor.assign(num_axis, 1.0);

        // Детерминированные локальные вариации загрязнения по осям (ТЗ, п.9)
        for (std::size_t i = 0; i < num_axis; ++i)
        {
            const double u = static_cast<double>(mix32(
                                 static_cast<std::uint32_t>(i) + 1u)) /
                    4294967296.0;
            axle_variation[i] = 0.7 + 0.6 * u;
        }
    }

    evolveSurface(dt, velocity, num_axis);

    // Буст песка затухает по таймеру
    for (std::size_t i = 0; i < num_axis; ++i)
    {
        if (sand_timer[i] > 0.0)
        {
            sand_timer[i] -= dt;
            if (sand_timer[i] <= 0.0)
                sand_boost[i] = 0.0;
        }
    }

    //--- Множитель сцепления ---

    // Влажность: мокрый рельс теряет до 35% сцепления
    double factor = 1.0 - 0.35 * wetness;

    // Загрязнение (листья/масло/грязь): до 45% на тяжёлом загрязнении
    factor -= 0.45 * std::min(contamination, 1.0);

    // Снег на рельсе: до 40%
    factor -= 0.40 * snow;

    // Лёд: до 70% (гололёд - минимальное сцепление, ТЗ, п.5)
    factor -= 0.70 * ice;

    // Скоростной эффект: плёнка воды при высокой скорости (ТЗ, п.14)
    const double speed_factor = 1.0 - 0.10 * wetness *
            std::min(std::abs(velocity) / 40.0, 1.0);
    factor *= speed_factor;

    // Состояние рельса (износ/ржавчина/шлифовка, ТЗ, п.17)
    switch (rail_condition)
    {
    case RailCondition::New:        factor *= 0.95; break; // новые - чуть хуже притёртых
    case RailCondition::Normal:                              break;
    case RailCondition::WavyWear:   factor *= 0.90; break;
    case RailCondition::Contaminated: factor *= 0.80; break;
    case RailCondition::Rusty:      factor *= 0.75; break;
    case RailCondition::Ground:     factor *= 1.05; break;
    }

    // Жара: сухие рельсы + пыль
    if (weather == Weather::ExtremeHeat)
        factor *= 0.92;

    factor = std::min(std::max(factor, 0.12), 1.0);

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        // Локальная вариация загрязнения по осям
        double axle = factor * (1.0 - 0.08 * (axle_variation[i] - 1.0));

        // Песок локально повышает сцепление оси (ТЗ, п.12)
        axle = std::min(axle + sand_boost[i], 1.15);

        axle_factor[i] = std::min(std::max(axle, 0.10), 1.15);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::evolveSurface(double dt, double velocity,
                                      std::size_t num_axis)
{
    const double abs_v = std::abs(velocity);

    // Осадки меняют состояние поверхности (ТЗ, п.3-6, 18)
    switch (weather)
    {
    case Weather::Dry:
    case Weather::ExtremeHeat:
        wetness -= dry_rate * dt * (temperature > 20.0 ? 2.0 : 1.0);
        snow -= 0.02 * dt;
        ice -= (temperature > 0.0) ? 0.01 * dt : 0.0;
        break;

    case Weather::Humid:
    case Weather::Fog:
        wetness += 0.001 * dt * weather_intensity;
        wetness = std::min(wetness, 0.4);
        break;

    case Weather::Drizzle:
        wetness += 0.0015 * dt * weather_intensity;
        break;

    case Weather::Rain:
        wetness += 0.003 * dt * weather_intensity;
        // Дождь смывает загрязнение (медленно, ТЗ, п.7)
        contamination -= 0.0005 * dt * weather_intensity;
        break;

    case Weather::HeavyRain:
        wetness += 0.006 * dt * weather_intensity;
        // Сильный дождь очищает быстрее, но плёнка воды толще
        contamination -= 0.002 * dt * weather_intensity;
        break;

    case Weather::Snow:
        snow += 0.008 * dt * weather_intensity;
        wetness += 0.01 * dt;
        break;

    case Weather::WetSnow:
        snow += 0.006 * dt * weather_intensity;
        wetness += 0.006 * dt * weather_intensity;
        break;

    case Weather::FreezingRain:
        wetness += 0.008 * dt * weather_intensity;
        ice += 0.004 * dt * weather_intensity;
        break;

    case Weather::Hoarfrost:
        ice += 0.003 * dt * weather_intensity;
        break;

    case Weather::Ice:
        ice += 0.015 * dt * weather_intensity;
        wetness -= 0.01 * dt;
        break;
    }

    // Температура: лёд тает выше нуля, влага испаряется на жаре (п.8)
    if (temperature > 2.0)
    {
        ice -= 0.01 * dt * (temperature - 2.0);
    }
    else if (temperature < -2.0)
    {
        // Замерзание влаги на рельсе: конвертация до 0.01/с
        const double freeze = std::min(wetness, 0.01 * dt);
        wetness -= freeze;
        ice += freeze;
    }

    // Самоочистка проходами колёсных пар (ТЗ, п.7): движущийся поезд
    // очищает рельсы; при продолжающихся осадках - медленнее
    if (abs_v > 0.5)
    {
        const double traffic = self_clean_rate * abs_v *
                static_cast<double>(num_axis) / 8.0;

        const double rain_slowdown = (weather == Weather::Rain ||
                                      weather == Weather::HeavyRain ||
                                      weather == Weather::Snow ||
                                      weather == Weather::WetSnow) ? 0.3 : 1.0;

        contamination -= traffic * dt * rain_slowdown;
        snow -= 1.5 * traffic * dt * rain_slowdown;
        ice -= 0.01 * traffic * dt * rain_slowdown;
    }

    // Органическое загрязнение медленно возвращается (листья/пыль)
    contamination += 0.00005 * dt;

    wetness = std::min(std::max(wetness, 0.0), 1.0);
    contamination = std::min(std::max(contamination, 0.0), 1.0);
    snow = std::min(std::max(snow, 0.0), 1.0);
    ice = std::min(std::max(ice, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getAxleFactor(std::size_t axle)
{
    if (axle >= axle_factor.size())
        return 1.0;

    return axle_factor[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getAverageFactor() const
{
    if (axle_factor.empty())
        return 1.0;

    double sum = 0.0;
    for (double value : axle_factor)
        sum += value;

    return sum / static_cast<double>(axle_factor.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelRailAdhesion::applySand(std::size_t axle, double boost,
                                  double duration)
{
    if (axle >= sand_boost.size())
        return;

    sand_boost[axle] = std::min(std::max(boost, 0.0), 0.5);
    sand_timer[axle] = std::max(sand_timer[axle], duration);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getWetness() const
{
    return wetness;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getContamination() const
{
    return contamination;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getIce() const
{
    return ice;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getHumidity() const
{
    return humidity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelRailAdhesion::getAirTemperature() const
{
    return temperature;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString WheelRailAdhesion::getDebugMsg() const
{
    return QString("Adhesion: avg factor %1, wet %2%, contam %3%, "
                   "snow %4%, ice %5%")
            .arg(getAverageFactor(), 0, 'f', 2)
            .arg(wetness * 100.0, 0, 'f', 0)
            .arg(contamination * 100.0, 0, 'f', 0)
            .arg(snow * 100.0, 0, 'f', 0)
            .arg(ice * 100.0, 0, 'f', 0);
}
