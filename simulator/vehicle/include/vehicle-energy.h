//------------------------------------------------------------------------------
//
//      Energy meter system (расход электроэнергии и статистика рейса)
//      ТЗ "Расход топлива и электроэнергии"
//
//      Энергетика считается от фактической механической работы колёс
//      (сила × скорость): P_el = P_mech / eta + P_aux. Цепь КПД
//      (трансформатор/выпрямитель/двигатели/редукторы) настраивается.
//      Рекуперация при электрическом торможении - отдельный счётчик
//      возврата с собственным КПД. Ток от напряжения КС: перегрузка по
//      току ограничивает доступную мощность (для тяговой системы).
//      Топливо тепловоза считает DieselEngineSystem (ТЗ #14).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_ENERGY_H
#define     VEHICLE_ENERGY_H

#include    <QString>

#include    <cstddef>

//------------------------------------------------------------------------------
/// Учёт энергии единицы ПС
//------------------------------------------------------------------------------
class EnergyMeterSystem
{
public:

    /// Род тяги
    enum class DriveType
    {
        Electric = 0,   ///< Электровоз/ЭПС от КС
        Diesel = 1,     ///< Тепловоз (учёт через дизель)
        Hybrid = 2      ///< Двухсистемный/гибрид
    };

    EnergyMeterSystem() = default;

    /// Загрузка секции [Energy]
    void loadConfig(QString cfg_path);

    /// Шаг: механическая сила тяги на ободе (Н, >0 тяга, <0 эл. тормоз),
    /// скорость (м/с), напряжение КС (В, 0 - нет КС), dt;
    /// network_regen_accept_w - сколько сеть готова принять (Вт),
    /// accepts - вообще ли сеть принимает рекуперацию
    void step(double dt, double traction_force, double velocity,
              double line_voltage, double network_regen_accept_w,
              bool network_accepts_regen);

    /// Потребляемая активная мощность, кВт
    double getPower() const;

    /// Ток из КС, А
    double getCurrent() const;

    /// Ограничение доступной мощности от тока/напряжения (0..1)
    double getPowerLimitFactor() const;

    /// Ограничение, применённое в последнем step() к учтённой мощности
    /// тяги (для отображения и будущей интеграции с ТЭД)
    double getLastPowerLimitFactor() const;

    /// Потреблено из КС, кВт*ч
    double getConsumed() const;

    /// Возвращено рекуперацией (принято сетью), кВт*ч
    double getRegenerated() const;

    /// Рассеято в реостатах (непринятая часть), кВт*ч
    double getDissipated() const;

    /// Тепловая загрузка тягового оборудования 0..1
    double getTractionThermal() const;

    /// Пройденный путь, км
    double getDistance() const;

    /// Удельный расход, кВт*ч/км
    double getSpecificConsumption() const;

    /// Пик потребления, кВт
    double getPeakPower() const;

    /// Сброс статистики рейса
    void resetTrip();

    QString getDebugMsg() const;

private:

    DriveType type = DriveType::Electric;

    /// Цепь КПД тяги: произведение (трансформатор × выпрямитель × ТЭД × редуктор)
    double traction_efficiency = 0.85;

    /// КПД рекуперации
    double regen_efficiency = 0.80;

    /// Мощность вспомогательных нужд, кВт
    double aux_power = 90.0;

    /// Номинальное напряжение КС, В
    double nominal_voltage = 25000.0;

    /// Предел тока, А (по токоприёмнику)
    double current_limit = 500.0;

    /// Максимальная рекуперативная мощность локомотива, кВт
    double max_regen_power = 4000.0;

    /// Минимальная скорость рекуперации, м/с
    double min_regen_speed = 5.0;

    /// Продолжительная мощность тяги (для тепловой модели), кВт
    double continuous_power = 0.6e3;

    /// Тепловая загрузка тягового оборудования 0..1
    double traction_thermal = 0.0;

    // Текущие значения
    double power_kw = 0.0;
    double current_a = 0.0;
    double limit_factor = 1.0;

    // Статистика рейса
    double consumed_kwh = 0.0;
    double regenerated_kwh = 0.0;
    double dissipated_kwh = 0.0;
    double distance_km = 0.0;
    double peak_power_kw = 0.0;
};

#endif // VEHICLE_ENERGY_H
