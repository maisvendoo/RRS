//------------------------------------------------------------------------------
//
//      Wheel-rail adhesion system (сцепление колёс с рельсами и погода)
//      ТЗ "Сцепление колёсных пар с рельсами" (SceplenieKpsRelsami)
//
//      Коэффициент сцепления - не константа: состояние поверхности рельса
//      (влажность, загрязнение, снег, лёд) эволюционирует под погодой и
//      самоочищается проходами колёсных пар. Коэффициент рассчитывается
//      на каждую ось отдельно (локальные вариации загрязнения).
//
//      Источник погоды будет подключён системой погоды (ТЗ "Видимость и
//      погода"); сейчас состояние задаётся API/сценарием.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_ADHESION_H
#define     VEHICLE_ADHESION_H

#include    <QString>

#include    <cstddef>
#include <vector>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Состояние сцепления колёс с рельсами для единицы ПС
//------------------------------------------------------------------------------
class VEHICLE_EXPORT WheelRailAdhesion
{
public:

    /// Погодные условия (ТЗ, п.2)
    enum class Weather
    {
        Dry = 0,            ///< Сухо
        Humid = 1,          ///< Высокая влажность
        Drizzle = 2,        ///< Морось
        Rain = 3,           ///< Дождь
        HeavyRain = 4,      ///< Сильный дождь
        Snow = 5,           ///< Снег
        WetSnow = 6,        ///< Мокрый снег / каша
        FreezingRain = 7,   ///< Ледяной дождь
        Hoarfrost = 8,      ///< Изморозь/иней
        Ice = 9,            ///< Гололёд
        Fog = 10,           ///< Туман (влажные рельсы)
        ExtremeHeat = 11    ///< Сильная жара (сухие рельсы, пыль)
    };

    /// Состояние поверхности рельса (ТЗ, п.17)
    enum class RailCondition
    {
        New = 0,        ///< Новые рельсы
        Normal = 1,     ///< Нормальный износ
        WavyWear = 2,   ///< Волнообразный износ
        Contaminated = 3,///< Загрязнённые
        Rusty = 4,      ///< Ржавые (редко используемые пути)
        Ground = 5      ///< Шлифованные
    };

    WheelRailAdhesion() = default;

    /// Загрузка секции [Adhesion]
    void loadConfig(QString cfg_path);

    /// Задать текущую погоду (интенсивность 0..1)
    void setWeather(Weather weather, double intensity);

    /// Задать температуру воздуха, град. C
    void setTemperature(double temperature);

    /// Шаг эволюции поверхности: dt, скорость ПЕ (для самоочистки
    /// проходами осей и скоростного эффекта), число осей ПЕ
    void step(double dt, double velocity, std::size_t num_axis);

    /// Множитель коэффициента сцепления для оси (ТЗ, п.9: оси различаются).
    /// Умножается на базовый psi колеса-рельса
    double getAxleFactor(std::size_t axle);

    /// Средний множитель (для диагностики)
    double getAverageFactor() const;

    /// Подать песок под ось (ТЗ, п.12): локальный временный рост сцепления
    void applySand(std::size_t axle, double boost, double duration);

    /// Влажность поверхности 0..1
    double getWetness() const;

    /// Загрязнение 0..1
    double getContamination() const;

    /// Лёд на поверхности 0..1
    double getIce() const;

    /// Заданная влажность воздуха 0..1
    double getHumidity() const;

    /// Заданная температура воздуха, град. C
    double getAirTemperature() const;

    QString getDebugMsg() const;

private:

    void evolveSurface(double dt, double velocity, std::size_t num_axis);

    Weather weather = Weather::Dry;
    double weather_intensity = 0.0;

    double temperature = 15.0;

    /// Влажность воздуха 0..1 (задаётся погодой)
    double humidity = 0.5;

    /// Состояние поверхности рельса
    double wetness = 0.0;
    double contamination = 0.1;
    double snow = 0.0;
    double ice = 0.0;

    /// Состояние рельса (износ) - постоянный множитель
    RailCondition rail_condition = RailCondition::Normal;

    /// Скорость самоочистки проходами осей (1/с на м/с скорости)
    double self_clean_rate = 0.004;

    /// Скорость высыхания, 1/с
    double dry_rate = 0.003;

    /// Локальный буст песка по осям (величина, таймер)
    std::vector<double> sand_boost;
    std::vector<double> sand_timer;

    /// Локальные вариации загрязнения по осям (детерминированные)
    std::vector<double> axle_variation;

    /// Кэш множителей по осям (обновляется в step)
    std::vector<double> axle_factor;
};

#endif // VEHICLE_ADHESION_H
