//------------------------------------------------------------------------------
//
//      Diesel engine system (тепловая модель дизеля, расход, дымность)
//      ТЗ "Реалистичная тепловая модель дизеля"
//
//      Взаимосвязанная модель: нагрузка -> обороты (регулятор с динамикой)
//      -> подача топлива -> мощность -> тепловыделение -> температуры
//      (ОЖ с термостатом и вентиляторами, масло, выхлоп) -> расход
//      топлива/масла -> дымность (чёрный/синий/белый) -> износ/неисправности.
//
//      Дополнительно (ТЗ, п.23, 24, 31): турбокомпрессор как объект
//      (обороты/температура/износ), воздушный фильтр, форсунки, утечки
//      топлива/масла/ОЖ, защиты аварийной остановки, холодный пуск,
//      вероятностные неисправности (пропуск вспышки, залипание рейки).
//
//      Привязка к тяге: система получает запрос мощности (setLoadDemand
//      0..1) от тяговой системы тепловоза; отклик мощности следует
//      за оборотами/турбонаддувом с задержкой.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_DIESEL_H
#define     VEHICLE_DIESEL_H

#include    <QString>

#include    <cstddef>
#include    <random>

//------------------------------------------------------------------------------
/// Дизельный двигатель тепловоза
//------------------------------------------------------------------------------
class DieselEngineSystem
{
public:

    /// Зона температуры охлаждающей жидкости (ТЗ, п.9)
    enum class TempZone
    {
        Normal = 0,
        Warm = 1,
        High = 2,
        Overheat = 3,
        Critical = 4
    };

    /// Дымность (ТЗ, п.12)
    enum class Smoke
    {
        None = 0,
        Light = 1,
        Medium = 2,
        Heavy = 3,
        Critical = 4
    };

    DieselEngineSystem() = default;

    /// Загрузка секции [Diesel]
    void loadConfig(QString cfg_path);

    /// Дизель есть на этой ПЕ (NominalPower > 0 в конфиге).
    /// Топливо/масло/ОЖ имеет смысл заправлять только ему
    bool isConfigured() const;

    /// Запуск / остановка дизеля
    void start();
    void stop();
    bool isRunning() const;

    /// Положение контроллера (задание оборотов), 0..1
    void setThrottle(double throttle);

    /// Запрос мощности от тяговой системы, 0..1
    void setLoadDemand(double demand);

    /// Шаг: dt, температура воздуха, скорость движения (обдув)
    void step(double dt, double air_temperature, double speed);

    //--------- Состояние ---------

    double getRPM() const;
    double getPower() const;            ///< кВт (фактическая)
    double getAvailablePowerFactor() const; ///< 0..1 (ограничения)

    double getCoolantTemperature() const;
    TempZone getTemperatureZone() const;
    double getOilTemperature() const;
    double getOilPressure() const;      ///< МПа
    double getExhaustTemperature() const;

    double getFuelLevel() const;        ///< доля 0..1
    double getFuelRate() const;         ///< л/ч
    double getTotalFuelConsumed() const;///< л
    double getSpecificFuel() const;     ///< г/кВт*ч

    double getOilLevel() const;
    double getCoolantLevel() const;     ///< доля 0..1
    double getTurboBoost() const;       ///< доля от максимального наддува

    Smoke getSmokeLevel() const;
    /// Цвет дыма: "none", "black", "blue", "white"
    QString getSmokeColor() const;

    double getWear() const;             ///< 0..1
    bool hasFault() const;

    /// Заправка топливом/маслом
    void refuel(double fuel_liters, double oil_liters);

    /// Доливка охлаждающей жидкости, л (без превышения ёмкости)
    void topUpCoolant(double liters);

    //--------- Турбокомпрессор (ТЗ, п.17, 31) ---------

    double getTurboRpm() const;         ///< об/мин (0..TurboMaxRpm)
    double getTurboTemp() const;        ///< град. C (нагрев от выхлопа)
    double getTurboWear() const;        ///< 0..1 (разнос/маслоголодание)

    //--------- Воздушный фильтр (ТЗ, п.31) ---------

    double getFilterClogging() const;   ///< 0..1
    void replaceFilter();               ///< обслуживание: новый фильтр

    //--------- Форсунки (ТЗ, п.31) ---------

    double getInjectorWear() const;     ///< 0..1 (агрегировано по цилиндрам)
    void serviceInjectors();            ///< замена форсунок

    //--------- Утечки (ТЗ, п.31) ---------

    double getFuelLeak() const;         ///< 0..1
    double getOilLeak() const;          ///< 0..1 (масло на асфальте - журнал)
    double getCoolantLeak() const;      ///< 0..1
    void repairLeaks();                 ///< устранение всех утечек

    //--------- Защиты и холодный пуск (ТЗ, п.23, 24) ---------

    bool isCranking() const;            ///< стартер крутит (холодный пуск)
    bool isProtectionTripped() const;   ///< сработала аваростан
    double getRestartDelay() const;     ///< с блокировки повторного пуска

    QString getDebugMsg() const;

private:

    void stepThermal(double dt, double power_kw, double air_temperature,
                     double speed);
    void stepCranking(double dt);
    void stepTurboShutdown(double dt);
    void tripProtection(const QString& reason);
    double coldStartProbability() const;
    double rand01();

    bool running = false;

    // Параметры
    double nominal_power = 1500.0;      ///< кВт
    double idle_rpm = 400.0;
    double max_rpm = 1100.0;
    double fuel_capacity = 5000.0;      ///< л
    double oil_capacity = 500.0;        ///< л

    /// Снабжение локомотива: система охлаждения (ТЗ "Снабжение")
    double coolant_capacity = 160.0;    ///< л
    double coolant_amount = 160.0;      ///< л

    /// Плотность дизельного топлива, кг/л
    static constexpr double fuel_density = 0.85;

    /// Максимальный расход через полностью открытую утечку, л/ч
    static constexpr double fuel_leak_max_flow = 300.0;
    static constexpr double oil_leak_max_flow = 50.0;
    static constexpr double coolant_leak_max_flow = 80.0;

    double t_warm = 65.0;               ///< прогрет
    double t_high = 90.0;
    double t_overheat = 97.0;
    double t_critical = 105.0;

    /// Температура воздуха, на которую откалиброван радиатор, град. C
    double radiator_ref_temp = 20.0;

    /// Запас ниже t_warm для белого дыма непрогретого дизеля, К
    double unwarmed_margin = 15.0;

    /// Удельный расход по нагрузке (г/кВт*ч): холостой, 0.25, 0.5, 0.75, 1.0
    double sfc_idle = 15.0;             ///< л/ч на холостом ходу
    double sfc_curve[5] = {260.0, 225.0, 210.0, 215.0, 235.0};

    // Состояние
    double throttle = 0.0;              ///< 0..1 контроллер
    double load_demand = 0.0;           ///< 0..1 запрос мощности
    double rpm = 0.0;
    double load = 0.0;                  ///< фактическая нагрузка 0..1
    double power_kw = 0.0;
    double available_power = 1.0;

    double fuel = 3000.0;               ///< л
    double fuel_rate = 0.0;             ///< л/ч
    double load_fuel_rate = 0.0;        ///< л/ч (нагрузочная часть без холостых)
    double fuel_total = 0.0;
    double oil = 450.0;
    double oil_rate = 0.0;              ///< л/ч

    double coolant_t = 20.0;
    double oil_t = 20.0;
    double exhaust_t = 20.0;
    double oil_pressure = 0.0;

    double turbo = 0.0;                 ///< 0..1
    double wear = 0.0;

    /// Признак зафиксированного критического уровня ОЖ (журнал)
    bool coolant_low_warned = false;

    Smoke smoke = Smoke::None;
    int smoke_color = 0;                ///< 0 none, 1 black, 2 blue, 3 white

    // Динамика регулятора/турбо
    double rpm_rate = 1.7;              ///< 1/с
    double turbo_rate = 0.8;            ///< 1/с

    //--------- Турбокомпрессор (ТЗ, п.31) ---------

    double turbo_max_rpm = 26000.0;     ///< об/мин
    double turbo_rpm = 0.0;             ///< об/мин (0..TurboMaxRpm)
    double turbo_t = 20.0;              ///< град. C
    double turbo_wear = 0.0;            ///< 0..1
    double turbo_stop_time = 100.0;     ///< с с момента остановки
                                        ///< (маслоголодание: первые 30 с)

    //--------- Воздушный фильтр ---------

    double filter_clog = 0.0;           ///< 0..1
    double filter_dust_rate = 0.002;    ///< 1/ч работы (пыльная погода
                                        ///  увеличивается извне через конфиг)
    bool filter_warned = false;         ///< событие при clog > 0.8

    //--------- Форсунки ---------

    double injector_wear = 0.0;         ///< 0..1
    double injector_wear_rate = 0.0001; ///< 1/ч работы под нагрузкой
    double fuel_quality = 1.0;          ///< 0.5..1 (плохое топливо -
                                        ///  ускоренный износ форсунок)
    double pulse_phase = 0.0;           ///< фаза пульсации холостого хода

    //--------- Утечки ---------

    double fuel_leak = 0.0;             ///< 0..1
    double oil_leak = 0.0;              ///< 0..1
    double coolant_leak = 0.0;          ///< 0..1
    double leak_probability = 0.02;     ///< 1/с при износе 1.0
    bool oil_spill_warned = false;      ///< "масло на асфальте" (журнал)

    //--------- Защиты аварийной остановки (ТЗ, п.23) ---------

    bool protections_enabled = true;
    double oil_pressure_min = 0.15;     ///< МПа (при вращении)
    double oil_temp_max = 110.0;        ///< град. C
    double overspeed_percent = 115.0;   ///< разнос
    double overspeed_time = 3.0;        ///< с до разрушения
    double anti_shutdown_time = 5.0;    ///< блокировка повторного пуска, с
    double protection_lock = 0.0;       ///< остаток блокировки, с
    double low_oil_pressure_timer = 0.0;
    double overspeed_timer = 0.0;
    bool overspeed_warned = false;

    //--------- Холодный пуск (ТЗ, п.24) ---------

    double cold_start_threshold = -10.0;    ///< град. C
    double cold_start_crank_time = 4.0;     ///< с прокрутки стартером
    double cold_start_reliability = 0.9;    ///< вероятность успеха на пороге
    bool cranking = false;
    double crank_timer = 0.0;
    double cold_stabilize_time = 0.0;       ///< замедление idle-регулятора, с
    double last_ambient = 20.0;             ///< температура воздуха из step()

    //--------- Вероятностные неисправности (ТЗ, п.31) ---------

    double misfire_probability = 0.05;      ///< 1/с при износе 1.0
    double stuck_rack_probability = 0.01;   ///< 1/с при износе 1.0
    double misfire_timer = 0.0;             ///< рывок момента 0.5 с
    double rack_stuck_timer = 0.0;          ///< залипание рейки 1..2 с

    /// ПСЧ сценария износа; seed из конфига (0 - случайный)
    std::mt19937 rng{std::random_device{}()};
};

#endif // VEHICLE_DIESEL_H
