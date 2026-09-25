//------------------------------------------------------------------------------
//
//      Weather system (погодные условия и видимость)
//
//------------------------------------------------------------------------------

#include    "weather-system.h"

#include    <CfgReader.h>

#include    <QDomNodeList>

#include    <algorithm>
#include <cmath>

namespace weather
{

namespace
{

/// Два пи (без зависимости от _USE_MATH_DEFINES)
constexpr double kTwoPi = 6.2831853071795865;

/// Час суток максимума суточного хода температуры (14:00)
constexpr double kDayPeakHour = 14.0;

/// Имя типа погоды -> тип (без учёта регистра и разделителей:
/// "heavy_rain", "HeavyRain" - одно и то же имя)
bool typeFromName(const QString& text, Type& type)
{
    QString name = text.toLower();
    name.remove('_');
    name.remove(' ');

    if (name == "dry" || name == "clear")  { type = Type::Dry; return true; }
    if (name == "humid")                   { type = Type::Humid; return true; }
    if (name == "drizzle")                 { type = Type::Drizzle; return true; }
    if (name == "rain")                    { type = Type::Rain; return true; }
    if (name == "heavyrain")               { type = Type::HeavyRain; return true; }
    if (name == "snow")                    { type = Type::Snow; return true; }
    if (name == "wetsnow")                 { type = Type::WetSnow; return true; }
    if (name == "freezingrain")            { type = Type::FreezingRain; return true; }
    if (name == "hoarfrost")               { type = Type::Hoarfrost; return true; }
    if (name == "ice")                     { type = Type::Ice; return true; }
    if (name == "fog")                     { type = Type::Fog; return true; }
    if (name == "extremeheat")             { type = Type::ExtremeHeat; return true; }

    // Расширенные типы (ТЗ "PogodniyeUsloviya"): пасмурно, гроза,
    // метель, густой туман ("overcast"/"heavyfog" - синонимы)
    if (name == "cloudy" || name == "overcast")
                                           { type = Type::Cloudy; return true; }
    if (name == "thunderstorm")            { type = Type::Thunderstorm; return true; }
    if (name == "blizzard")                { type = Type::Blizzard; return true; }
    if (name == "densefog" || name == "heavyfog")
                                           { type = Type::DenseFog; return true; }

    return false;
}

Type typeFromString(const QString& text)
{
    Type type = Type::Dry;
    typeFromName(text, type);
    return type;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WeatherSystem::load(const QString& route_dir)
{
    CfgReader cfg;

    if (!cfg.load(route_dir + "/weather.conf"))
    {
        // Умеренная ясная погода по умолчанию
        return;
    }

    QString type_str = "";
    if (cfg.getString("Weather", "Type", type_str))
        current_.type = typeFromString(type_str);

    cfg.getDouble("Weather", "Intensity", current_.intensity);
    cfg.getDouble("Weather", "Temperature", current_.temperature);
    cfg.getDouble("Weather", "Humidity", current_.humidity);
    cfg.getDouble("Weather", "WindSpeed", current_.wind_speed);
    cfg.getDouble("Weather", "WindDirection", current_.wind_direction);
    cfg.getDouble("Weather", "TransitionTime", transition_time_);

    // Суточный ход температуры (ТЗ, п.6): ключ DayNightCycle=true,
    // амплитуда DayNightSwing (по умолчанию +/-5 град. C)
    cfg.getBool("Weather", "DayNightCycle", day_night_cycle_);
    cfg.getDouble("Weather", "DayNightSwing", day_night_swing_);
    day_night_swing_ = std::min(std::max(day_night_swing_, 0.0), 20.0);

    current_.intensity = std::min(std::max(current_.intensity, 0.0), 1.0);

    // Базы видимости по типам погоды без хардкода (ТЗ): секция
    // [Visibility], для каждого типа - ключ=значение (HeavyRain=2000).
    // Отсутствующие ключи остаются с дефолтами
    const QDomNode vis_node = cfg.getFirstSection("Visibility");

    if (!vis_node.isNull())
    {
        const QDomNodeList keys = vis_node.childNodes();

        for (int i = 0; i < keys.size(); ++i)
        {
            const QDomElement key = keys.item(i).toElement();

            if (key.isNull())
                continue;

            Type type = Type::Dry;
            if (!typeFromName(key.tagName(), type))
                continue;

            bool ok = false;
            const double base = key.text().toDouble(&ok);

            if (ok && base >= 50.0)
                visibility_base_[type] = base;
        }
    }

    // Локальные зоны тумана (ТЗ, п.9): повторяемые секции [FogZone] с
    // пикетажем Begin/End и видимостью внутри зоны Visibility
    QDomNode fog_node = cfg.getFirstSection("FogZone");

    while (!fog_node.isNull())
    {
        const QDomElement elem = fog_node.toElement();

        if (!elem.isNull() &&
                elem.tagName().compare("FogZone", Qt::CaseInsensitive) == 0)
        {
            FogZone zone;

            cfg.getDouble(fog_node, "Begin", zone.begin);
            cfg.getDouble(fog_node, "End", zone.end);
            cfg.getDouble(fog_node, "Visibility", zone.visibility);

            // Осмысленная зона: ненулевой участок и видимость не ниже
            // минимальной по ТЗ (50 м)
            if (zone.end > zone.begin && zone.visibility >= 50.0)
                fog_zones_.push_back(zone);
        }

        fog_node = cfg.getNextSection();
    }

    // Высотный туман (ТЗ, п.10): секция [HeightFog] - Height (H0, м)
    // и Multiplier (множитель плотности у земли)
    const QDomNode hf_node = cfg.getFirstSection("HeightFog");

    if (!hf_node.isNull())
    {
        cfg.getBool(hf_node, "Enabled", height_fog_enabled_);
        cfg.getDouble(hf_node, "Height", height_fog_height_);
        cfg.getDouble(hf_node, "Multiplier", height_fog_multiplier_);

        height_fog_height_ = std::max(height_fog_height_, 1.0);
        height_fog_multiplier_ = std::max(height_fog_multiplier_, 1.0);
    }

    applyVisibility(current_);

    // База температуры (без суточной добавки)
    temperature_base_ = current_.temperature;

    target_ = current_;
    start_ = current_;
    transition_progress_ = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WeatherSystem::setTarget(Type type, double intensity,
                              double transition_time)
{
    target_.type = type;
    target_.intensity = std::min(std::max(intensity, 0.0), 1.0);

    // Температуру целевого состояния не меняем - её задаёт
    // сценарий отдельно через конфиг

    // Ветер при грозе/метели (ТЗ "PogodniyeUsloviya"): усиление
    // пропорционально интенсивности - до 25 м/с (гроза) и 15 м/с (метель)
    switch (target_.type)
    {
    case Type::Thunderstorm:
        target_.wind_speed = std::max(target_.wind_speed,
                                      25.0 * target_.intensity);
        break;

    case Type::Blizzard:
        target_.wind_speed = std::max(target_.wind_speed,
                                      15.0 * target_.intensity);
        break;

    default:
        break;
    }

    if (transition_time > 0.0)
        transition_time_ = transition_time;

    applyVisibility(target_);

    start_ = current_;

    // Стартовая база температуры - без суточной добавки, иначе она
    // "утащила" бы добавку в интерполяцию
    start_.temperature = temperature_base_;

    transition_progress_ = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WeatherSystem::step(double dt)
{
    // Часы симуляции: внешнее время (setTime) приоритетнее,
    // иначе часы идут от собственного dt
    if (!time_set_)
        sim_hours_ += dt / 3600.0;

    // Суточный ход температуры (ТЗ, п.6): синус с максимумом в 14:00,
    // минимумом в 02:00; амплитуда DayNightSwing (по умолчанию 5 град.)
    daynight_delta_ = day_night_cycle_
            ? day_night_swing_ *
              std::sin(kTwoPi * (sim_hours_ - (kDayPeakHour - 6.0)) / 24.0)
            : 0.0;

    // Плавный переход (ТЗ, п.5): линейная интерполяция от стартового
    // состояния к целевому по прогрессу
    if (transition_progress_ < 1.0)
    {
        const double rate = dt / std::max(transition_time_, 1.0);

        transition_progress_ = std::min(1.0, transition_progress_ + rate);

        const double k = transition_progress_;

        // Тип погоды переключается только В КОНЦЕ перехода: до этого
        // числовые параметры (интенсивность/видимость/туман) интерполируются
        // от старого состояния к целевому, а дискретный тип остаётся прежним
        if (transition_progress_ >= 1.0)
            current_.type = target_.type;

        current_.intensity = start_.intensity +
                (target_.intensity - start_.intensity) * k;

        current_.visibility = start_.visibility +
                (target_.visibility - start_.visibility) * k;

        current_.fog_density = start_.fog_density +
                (target_.fog_density - start_.fog_density) * k;

        // Ветер (усиление при грозе/метели) переводится так же плавно
        current_.wind_speed = start_.wind_speed +
                (target_.wind_speed - start_.wind_speed) * k;

        temperature_base_ = start_.temperature +
                (target_.temperature - start_.temperature) * k;
    }

    // Эффективная температура = база + суточная добавка
    current_.temperature = temperature_base_ + daynight_delta_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WeatherSystem::setTime(double hours)
{
    // Время суток, ч: задаётся потребителем (моделью) на каждом тике
    // или реже; достаточно один раз - дальше часы идут от dt
    sim_hours_ = hours;
    time_set_ = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WeatherSystem::applyVisibility(State& state)
{
    // База видимости данного типа: из конфига, при отсутствии ключа -
    // дефолты ТЗ (заданы в visibility_base_)
    const auto it = visibility_base_.find(state.type);
    const double base = (it != visibility_base_.end()) ? it->second
                                                       : 15000.0;

    // Слабые осадки видны дальше сильных: сильный снегопад при высокой
    // интенсивности даёт 100-700 м (база * (1 - 0.6*i))
    const double k = 1.0 - 0.6 * state.intensity;

    state.visibility = std::max(base * k, 50.0);

    // Плотность тумана: экспоненциальное затухание видимости
    state.fog_density = 3.9 / std::max(state.visibility, 50.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const State& WeatherSystem::getState() const
{
    return current_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WeatherSystem::getVisibility() const
{
    return current_.visibility;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WeatherSystem::getVisibilityAt(double coord) const
{
    // Видимость в точке маршрута (ТЗ, п.9): глобальная, модифицированная
    // зонами локального тумана. Внутри зоны - min(глобальная, зональная),
    // на границах - линейный фаде длиной 200 м
    double visibility = current_.visibility;

    for (const FogZone& zone : fog_zones_)
    {
        if (coord < zone.begin || coord > zone.end)
            continue;

        // Глубина внутри зоны от ближайшей границы, м
        const double depth = std::min(coord - zone.begin,
                                      zone.end - coord);

        const double k = std::min(std::max(depth / fog_fade_, 0.0), 1.0);
        const double in_zone = std::min(visibility, zone.visibility);

        // Фаде: от глобальной видимости на границе до зональной в глубине
        visibility = visibility * (1.0 - k) + in_zone * k;
    }

    return visibility;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WeatherSystem::getFogDensity() const
{
    return current_.fog_density;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WeatherSystem::getFogDensityAt(double height) const
{
    // Высотный туман (ТЗ, п.10): плотность растёт экспоненциально
    // ниже высоты H0. Множитель на глубине d ниже H0:
    // m = Multiplier^(d/H0) - у земли Multiplier, на H0 и выше - 1
    if (!height_fog_enabled_)
        return current_.fog_density;

    const double depth = std::min(
                std::max((height_fog_height_ - height) / height_fog_height_,
                         0.0),
                1.0);

    const double power = depth * std::log(height_fog_multiplier_);

    return current_.fog_density * std::exp(power);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool WeatherSystem::isSnowfall() const
{
    // Признак снегопада для стекла кабины и аудио: снежные типы погоды,
    // включая метель (Blizzard)
    switch (current_.type)
    {
    case Type::Snow:
    case Type::WetSnow:
    case Type::Blizzard:
        return true;

    default:
        return false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Type WeatherSystem::adhesionEquivalent(Type type)
{
    // Таблица соответствия расширенных типов эффектам сцепления
    // (WheelRailAdhesion::Weather в vehicle-adhesion.h не расширяется):
    // потребитель (модель) подхватит позже
    switch (type)
    {
    case Type::Cloudy:
        return Type::Dry;           // пасмурно - сухой рельс

    case Type::Thunderstorm:
        return Type::HeavyRain;     // ливень грозы

    case Type::Blizzard:
        return Type::Snow;          // снег с ветром

    case Type::DenseFog:
        return Type::Fog;           // влажные рельсы

    default:
        return type;                // остальные имеют прямой аналог
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Type WeatherSystem::getAdhesionEquivalent() const
{
    return adhesionEquivalent(current_.type);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::vector<FogZone>& WeatherSystem::getFogZones() const
{
    return fog_zones_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString WeatherSystem::getDebugMsg() const
{
    return QString("Weather: type %1, intensity %2, visibility %3 m, "
                   "fog %4, wind %5 m/s, T %6 C")
            .arg(static_cast<int>(current_.type))
            .arg(current_.intensity, 0, 'f', 2)
            .arg(current_.visibility, 0, 'f', 0)
            .arg(current_.fog_density, 0, 'e', 5)
            .arg(current_.wind_speed, 0, 'f', 1)
            .arg(current_.temperature, 0, 'f', 1);
}

} // namespace weather
