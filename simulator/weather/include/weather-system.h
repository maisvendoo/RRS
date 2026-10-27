//------------------------------------------------------------------------------
//
//      Weather system (погодные условия и видимость)
//      ТЗ "PogodniyeUsloviya" (видимость/туман/осадки/ветер)
//
//      Ядро состояния погоды: текущее и целевое состояние, плавный
//      переход (интерполяция интенсивности), дальность видимости,
//      плотность тумана, ветер (для токоприёмников и КС). Питает
//      систему сцепления (WheelRailAdhesion) и токоприёмники.
//      Рендер (туман/небо/осадки) во вьювере берёт параметры отсюда.
//
//      Дополнительно (ТЗ, п.6/9/10): суточный ход температуры,
//      локальные зоны тумана по пикетажу и высотный туман.
//
//------------------------------------------------------------------------------

#ifndef     WEATHER_SYSTEM_H
#define     WEATHER_SYSTEM_H

#include    "weather-export.h"

#include    <QString>

#include    <map>
#include    <vector>

namespace weather
{

/// Тип погоды: значения 0..11 согласованы с WheelRailAdhesion::Weather;
/// расширенные типы 12..15 не имеют прямого аналога там и переводятся
/// в эффект сцепления методом adhesionEquivalent()
enum class Type
{
    Dry = 0,
    Humid = 1,
    Drizzle = 2,
    Rain = 3,
    HeavyRain = 4,
    Snow = 5,
    WetSnow = 6,
    FreezingRain = 7,
    Hoarfrost = 8,
    Ice = 9,
    Fog = 10,
    ExtremeHeat = 11,

    //--- Расширенные типы (ТЗ "PogodniyeUsloviya") ---

    Cloudy = 12,        ///< Пасмурно/облачность: без осадков, видимость 8 км
    Thunderstorm = 13,  ///< Гроза: ливень + ветер до 25 м/с, видимость 800 м
    Blizzard = 14,      ///< Метель: снег + ветер 15 м/с, видимость 300 м
    DenseFog = 15       ///< Густой туман: видимость 50 м
};

/// Состояние погоды в точке
struct State
{
    Type type = Type::Dry;
    double intensity = 0.0;     ///< 0..1
    double visibility = 15000.0;///< Дальность видимости, м
    double fog_density = 0.0;   ///< Плотность тумана, 1/м
    double wind_speed = 0.0;    ///< Скорость ветра, м/с
    double wind_direction = 0.0;///< Азимут ветра, рад
    double temperature = 15.0;  ///< Температура воздуха, град. C
    double humidity = 0.5;      ///< Влажность 0..1
};

/// Зона локального тумана (ТЗ, п.9): участок маршрута с пониженной
/// видимостью (низина, болото, мост через реку). Внутри зоны видимость
/// = min(глобальная, зональная), на границах - плавный фаде
struct FogZone
{
    double begin = 0.0;         ///< Начало зоны, м (пикетаж маршрута)
    double end = 0.0;           ///< Конец зоны, м
    double visibility = 50.0;   ///< Видимость внутри зоны, м
};

//------------------------------------------------------------------------------
/// Система погоды симуляции
//------------------------------------------------------------------------------
class WEATHER_EXPORT WeatherSystem
{
public:

    WeatherSystem() = default;

    /// Загрузка конфига маршрута (weather.conf): секции [Weather],
    /// [Visibility], [FogZone] (локальный туман, п.9 ТЗ),
    /// [HeightFog] (высотный туман, п.10 ТЗ)
    void load(const QString& route_dir);

    /// Задать целевое состояние (сценарий/диспетчер): система плавно
    /// перейдёт к нему за transition_time секунд. Для грозы/метели
    /// усиливает ветер целевого состояния (до 25/15 м/с)
    void setTarget(Type type, double intensity, double transition_time);

    /// Шаг (продвигает собственные часы симуляции, если время не
    /// задано внешним setTime())
    void step(double dt);

    /// Задать текущее время суток симуляции, ч (0..24): для суточного
    /// хода температуры (ТЗ, п.6). Вызывается потребителем (моделью);
    /// без вызова часы идут от dt. Сам цикл включается ключом
    /// DayNightCycle конфига (по умолчанию выключен)
    void setTime(double hours);

    /// Текущее состояние (температура уже с суточной добавкой)
    const State& getState() const;

    /// Видимость, м (глобальная, без зон тумана)
    double getVisibility() const;

    /// Видимость в точке маршрута, м (ТЗ, п.9): глобальная с учётом
    /// зон локального тумана (внутри зоны = min(глобальная, зональная),
    /// на границах плавный фаде 200 м)
    double getVisibilityAt(double coord) const;

    /// Плотность тумана (базовая, на уровне пути), 1/м
    double getFogDensity() const;

    /// Плотность тумана на высоте, 1/м (ТЗ, п.10): ниже высоты H0
    /// плотность растёт экспоненциально (у земли - базовая * Multiplier,
    /// на H0 и выше - базовая)
    double getFogDensityAt(double height) const;

    /// Идёт снегопад (Snow/WetSnow/Blizzard): признак для
    /// обработки стекла кабины и аудио
    bool isSnowfall() const;

    /// Таблица соответствия типов погоды эффектам сцепления
    /// (WheelRailAdhesion::Weather): расширенные типы переводятся в
    /// ближайший существующий (Thunderstorm->HeavyRain,
    /// Blizzard->Snow, DenseFog->Fog, Cloudy->Dry). Потребитель
    /// (модель) подхватит позже
    static Type adhesionEquivalent(Type type);

    /// Эквивалент текущей погоды для системы сцепления
    Type getAdhesionEquivalent() const;

    /// Зоны локального тумана (для отладки/рендера)
    const std::vector<FogZone>& getFogZones() const;

    QString getDebugMsg() const;

private:

    State current_;
    State target_;
    State start_;                       ///< Состояние на старте перехода

    double transition_time_ = 120.0;    ///< Время перехода, с
    double transition_progress_ = 1.0;  ///< 1 - переход завершён

    /// Базы видимости по типам погоды, м (при полной интенсивности).
    /// Дефолты - по ТЗ, переопределяются секцией [Visibility] weather.conf
    std::map<Type, double> visibility_base_
    {
        {Type::Dry, 15000.0},
        {Type::Humid, 15000.0},
        {Type::ExtremeHeat, 15000.0},
        {Type::Drizzle, 8000.0},
        {Type::Rain, 5000.0},
        {Type::HeavyRain, 2000.0},
        {Type::Snow, 1500.0},
        {Type::WetSnow, 1000.0},
        {Type::FreezingRain, 3000.0},
        {Type::Hoarfrost, 6000.0},
        {Type::Ice, 10000.0},
        {Type::Fog, 250.0},
        {Type::Cloudy, 8000.0},
        {Type::Thunderstorm, 800.0},
        {Type::Blizzard, 300.0},
        {Type::DenseFog, 50.0}
    };

    //--- Локальный туман (ТЗ, п.9) ---

    std::vector<FogZone> fog_zones_;    ///< Зоны тумана маршрута
    double fog_fade_ = 200.0;           ///< Длина фаде на границах зоны, м

    //--- Высотный туман (ТЗ, п.10) ---

    bool height_fog_enabled_ = false;   ///< Ключ [HeightFog] Enabled
    double height_fog_height_ = 40.0;   ///< H0: ниже этой высоты гуще, м
    double height_fog_multiplier_ = 8.0;///< Множитель плотности у земли

    //--- Суточный ход температуры (ТЗ, п.6) ---

    bool day_night_cycle_ = false;      ///< Ключ [Weather] DayNightCycle
    double day_night_swing_ = 5.0;      ///< Амплитуда +/-, град. C
    double temperature_base_ = 15.0;    ///< База температуры (без суточной добавки)
    double daynight_delta_ = 0.0;       ///< Текущая суточная добавка, град. C
    double sim_hours_ = 12.0;           ///< Часы симуляции (0..24)
    bool time_set_ = false;             ///< Время задано внешним setTime()

    /// Параметры видимости по типу погоды (метры при полной интенсивности)
    void applyVisibility(State& state);
};

} // namespace weather

#endif // WEATHER_SYSTEM_H
