//------------------------------------------------------------------------------
//
//      Condensate system (влага и лёд в пневматической системе)
//      ТЗ "RRS: конденсат, температура, влажность, пневмосистема"
//
//      Ядро физики влажности ПЕ: точка росы по Магнусу от температуры
//      и влажности воздуха (подаёт погода через систему сцепления),
//      температура резервуаров с тепловой инерцией и нагревом от работы
//      компрессора, накопление влаги при зарядке магистрали, конденсация
//      на холодных резервуарах, замерзание/оттаивание воды, слив
//      конденсата и авто-продувка на стоянке.
//
//      Влияние на тормоза: лёд сужает проходные сечения и замедляет
//      тормозную волну - экспортируется getBrakeResponseFactor()
//      0.5..1.0 (потребитель - тормозные устройства; интеграция
//      выполняется моделью тормозов).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_CONDENSATE_H
#define     VEHICLE_CONDENSATE_H

#include    <QString>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Конденсат и лёд в пневматической системе единицы ПС
//------------------------------------------------------------------------------
class VEHICLE_EXPORT CondensateSystem
{
public:

    CondensateSystem() = default;

    /// Загрузка секции [PneumoCondensate]
    void loadConfig(QString cfg_path);

    /// Шаг.
    /// dt - интервал (термо-тик); air_temperature - град. C;
    /// relative_humidity - 0..1; air_charging - система под давлением
    /// (компрессор работает/магистраль заряжается - воздух с влагой
    /// поступает в резервуары); standing - ПЕ стоит (авто-продувка)
    void step(double dt,
              double air_temperature,
              double relative_humidity,
              bool air_charging,
              bool standing);

    //--------- Диагностика (ТЗ, п.18) ---------

    /// Точка росы по Магнусу от последних входных данных, град. C
    double getDewPoint() const;

    /// Температура воздуха (последний вход), град. C
    double getAirTemperature() const;

    /// Температура главных резервуаров, град. C (с инерцией и
    /// подогревом от работы компрессора)
    double getReservoirTemperature() const;

    /// Влажность воздуха в пневмосистеме (PneumaticMoisture) 0..1
    double getMoisture() const;

    /// Жидкий конденсат в резервуарах 0..1 (нормировано на типовой объём)
    double getLiquidWater() const;

    /// Доля конденсата, замёрзшая в лёд, 0..1
    double getFrozenFraction() const;

    /// Суммарный лёд (жидкая вода * доля замерзания) 0..1
    double getIceAmount() const;

    /// Пневмосистема существенно обледенела
    bool isFrozen() const;

    /// Множитель скорости тормозной волны 0.5..1.0: лёд в рукавах,
    /// кранах и воздухораспределителях замедляет распространение
    /// давления (ТЗ, п.13). 1.0 - система сухая
    double getBrakeResponseFactor() const;

    //--------- Обслуживание (ТЗ, п.15) ---------

    /// Открыть сливной кран главных резервуаров
    void drainValve();

    /// Закрыть сливной кран
    void closeDrainValve();

    /// Сливной кран открыт
    bool isDrainValveOpen() const;

    QString getDebugMsg() const;

    /// Статическая точка росы по формуле Магнуса, град. C
    /// (T - температура воздуха, phi - относительная влажность 0..1)
    static double dewPointMagnus(double T, double phi);

private:

    bool enabled = true;

    /// Скорость роста влажности при зарядке, 1/с (умножается на
    /// влажность воздуха: влажный день - быстрее)
    double moisture_rate = 0.002;

    /// Температура замерзания, град. C
    double freeze_threshold = 0.0;

    /// Скорость слива конденсата, доля/с
    double drain_rate = 0.05;

    /// Автоматическая продувка на стоянке
    bool auto_drain = false;

    /// Скорость замерзания воды, 1/с
    double freeze_rate = 0.02;

    /// Скорость оттаивания при T выше нуля, 1/с (умножается на прогрев)
    double thaw_rate = 0.01;

    /// Тепловая инерция резервуаров, с
    double reservoir_tau = 1800.0;

    /// Подогрев резервуаров от работы компрессора, К
    double compressor_heating = 8.0;

    /// Скорость конденсации на холодном резервуаре, 1/с на К разницы
    double condensation_rate = 0.002;

    // Состояние
    double air_temp = 15.0;             ///< последний вход, град. C
    double humidity = 0.5;              ///< последний вход 0..1
    double dew_point = 4.4;             ///< кэш точки росы
    double reservoir_t = 15.0;          ///< температура резервуаров
    double moisture = 0.0;              ///< PneumaticMoisture 0..1
    double liquid = 0.0;                ///< жидкий конденсат 0..1
    double frozen_fraction = 0.0;       ///< доля замёрзшей воды 0..1
    bool drain_open = false;

    /// Предупреждение об обледенении (журнал, однократно)
    bool freeze_warned = false;
};

#endif // VEHICLE_CONDENSATE_H
