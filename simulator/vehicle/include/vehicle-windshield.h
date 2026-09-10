//------------------------------------------------------------------------------
//
//      Windshield system (динамическое загрязнение лобового стекла)
//      ТЗ "Динамическое загрязнение лобового стекла"
//
//      Грязь/плёнка воды накапливаются от осадков (сильнее на скорости),
//      брызг из-под колёс на мокром пути и грязи; дворники чистят
//      пройденную зону (износ оставляет полосы), омыватель смывает,
//      зимой влага замерзает в лёд (дворники бессильны, нужен обогрев).
//      Датчик: коэффициент видимости из кабины 0..1.
//
//      Дополнительно (ТЗ, п.2-3, 8-10): капли-частицы с изменяемым
//      радиусом (слияние близких), зоны очистки дворников (капли и
//      грязь вне зоны остаются), режимы INT/низкий/высокий,
//      авто-дворники, задержка дворников после омывателя,
//      снежная плёнка отдельно от воды.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_WINDSHIELD_H
#define     VEHICLE_WINDSHIELD_H

#include    <QString>

#include    <cstddef>
#include    <cstdint>
#include    <vector>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Лобовое стекло кабины
//------------------------------------------------------------------------------
class VEHICLE_EXPORT WindshieldSystem
{
public:

    /// Лимит капель-частиц (ТЗ, п.2)
    static constexpr std::size_t kMaxDroplets = 256;

    /// Капля воды на стекле (частица): рендер берёт массив через
    /// getDroplets(), физика (стекание/слияние) - внутри step()
    struct Droplet
    {
        float u = 0.5f;         ///< Горизонталь на стекле 0..1
        float v = 0.9f;         ///< Вертикаль 0..1 (0 - низ стекла)
        float radius = 1.0f;    ///< Радиус, мм (скорость стекания пропорциональна ему)
        float life = 10.0f;     ///< Время жизни, с
    };

    WindshieldSystem() = default;

    /// Загрузка секции [Windshield]
    void loadConfig(QString cfg_path);

    /// Дворники: 0 - OFF, 1 - INT (пауза от интенсивности осадков),
    /// 2 - постоянный низкий (1 Гц), 3 - постоянный высокий (2 Гц)
    void setWipers(int mode);

    /// Омыватель (кратковременный импульс): расход на удар 0.02 бака,
    /// дворники срабатывают через 0.7 с (пара взмахов)
    void wash();

    /// Обогрев стекла
    void setDefroster(bool on);

    /// Шаг: скорость ПЕ, интенсивность осадков 0..1, мокрота пути 0..1,
    /// температура воздуха, идёт снегопад (снежная плёнка вместо капель;
    /// по умолчанию выкл - совместимость со старыми вызывающими)
    void step(double dt, double velocity, double rain_intensity,
              double track_wetness, double air_temperature,
              bool snowing = false);

    /// Загрязнённость стекла 0..1
    double getDirt() const;

    /// Лёд на стекле 0..1
    double getIce() const;

    /// Снежная плёнка 0..1 (накапливается при снегопаде, чистится
    /// дворниками хуже x0.5, плавится обогревом)
    double getSnowFilm() const;

    /// Капли-частицы для рендера (не больше kMaxDroplets)
    const std::vector<Droplet>& getDroplets() const;

    /// Коэффициент видимости из кабины 0..1 (чистое = 1)
    double getVisibilityFactor() const;

    /// Жидкость омывателя, доля 0..1
    double getWasherFluid() const;

    /// Износ дворников 0..1
    double getWiperWear() const;

    /// Текущий режим дворников 0..3
    int getWipers() const;

    QString getDebugMsg() const;

private:

    /// Спавн новых капель от встречного потока осадков
    void spawnDroplets(double dt, double rain_intensity,
                       double speed_factor);

    /// Стекание/слияние/испарение капель (скорость стекания ∝ радиусу)
    void stepDroplets(double dt, double speed_factor);

    /// Такт дворников: счёт взмахов по частоте режима (0.5/1/2 Гц)
    void stepWipers(double dt, double rain_intensity, double ice);

    /// Взмах дворника: снимает капли/грязь/снег в зоне очистки
    /// (сектор), вне зоны - остаются (реалистично, ТЗ, п.3)
    void wiperStroke(double ice);

    /// Капля/точка в зоне очистки дворника (сектор: центр + угол + вылет)
    bool isInWiperZone(float u, float v) const;

    /// Детерминированный ГПСЧ (xorshift32) для спавна капель
    float nextFloat();

    double dirt = 0.05;             ///< Загрязнение 0..1
    double water_film = 0.0;        ///< Плёнка воды 0..1
    double ice = 0.0;               ///< Лёд 0..1
    double snow_film = 0.0;         ///< Снежная плёнка 0..1
    double washer_fluid = 1.0;      ///< Омыватель 0..1
    double wiper_wear = 0.0;        ///< Износ щёток 0..1

    int wiper_mode = 0;             ///< 0..3
    double wash_timer = 0.0;
    bool defroster = false;

    //--- Зона очистки дворников (ТЗ, п.3) ---

    double wiper_arc = 110.0;       ///< Угол сектора очистки, град
    double wiper_center_u = 0.5;    ///< Ось дворника, горизонталь 0..1
    double wiper_center_v = 0.0;    ///< Ось дворника, вертикаль (низ стекла)
    double wiper_length = 0.55;     ///< Вылет щётки (доля ширины стекла)

    //--- Такт дворников ---

    double wiper_phase = 0.0;       ///< Фаза цикла взмаха (отрицательная = пауза INT)
    double wiper_delay_timer = 0.0; ///< Задержка срабатывания после омывателя, с
    int wash_wipes_pending = 0;     ///< Осталось взмахов после омывателя

    //--- Авто-режим (ключ AutoWipers) ---

    bool auto_wipers = false;       ///< Автовключение INT при воде > 0.3
    bool auto_engaged = false;      ///< INT включён автоматически (не игроком)

    //--- Капли-частицы ---

    std::vector<Droplet> droplets_; ///< Активные капли (до kMaxDroplets)
    double drop_spawn_acc = 0.0;    ///< Дробный аккумулятор спавна, капель/с
    std::uint32_t rng = 2017723u;   ///< Состояние ГПСЧ спавна

    //--- Скорости процессов (секция [Windshield]) ---

    double splash_rate = 0.02;      ///< Скорость накопления грязи от брызг

    double rain_rate = 0.03;        ///< Скорость осаждения осадков

    double snow_rate = 0.02;        ///< Скорость накопления снежной плёнки

    double wiper_clean_rate = 0.25; ///< Скорость очистки дворниками (на 1 режим)

    double wash_use = 0.02;         ///< Расход омывателя на удар (доля бака)

    double wiper_delay = 0.7;       ///< Задержка дворников после омывателя, с

    double defrost_rate = 0.05;     ///< Скорость таяния льда обогревом
};

#endif // VEHICLE_WINDSHIELD_H
