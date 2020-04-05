#ifndef     DISEL_H
#define     DISEL_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Disel : public Device
{
public:

    Disel(QObject *parent = Q_NULLPTR);

    ~Disel();

    void step(double t, double dt);

    /// Задать расход масла от электрического маслопрокачивающего насоса (ЭМН)
    void setQ_emn(double Q_emn);

    /// Задать крутящий момент от стартер-генератора
    void setStarterTorque(double M_sg);

    /// Задать состояние МВ6
    void setMV6state(bool state_mv6) { this->state_mv6 = state_mv6; }

    /// Задать состояние ВНТ
    void setVTNstate(bool state_vtn) { this->state_vtn = state_vtn; }

    /// Задать давление топлива перед ТНВД
    void setFuelPressure(double fuel_pressure) { this->fuel_pressure = fuel_pressure; }

    /// Вернуть давление в системе смазки
    double getOilPressure() const { return getY(0); }

    /// Вернуть частоту вращения привода стартер-генератора
    double getStarterOmega() const { return ip * getY(1); }

    /// Вернуть частоту вращения тягового генератора
    double getOmega() const { return getY(1); }

    /// Вернуть фактический расход топлива
    double getFuelFlow() const { return Q_fuel; }

private:

    enum
    {
        NUM_COEFFS = 10,

        /// Давление масла
        OIL_PRESSURE = 0,

        /// Угловая скорость вращения вала
        SHAFT_OMEGA = 1,
    };

    /// Условный объем маслянного русла
    double  V_oil;

    /// Расход масла от ЭМН
    double  Q_emn;

    /// Момент от стартер-генератора
    double  M_sg;

    /// Момент сопротивления на валу
    double  Mc;

    /// Передаточное число привода стартер-генератора
    double  ip;

    /// Приведенный к коленчатому валу момент инерции (условно постоянный!)
    double  J_shaft;

    /// Признак воспламенения топлива
    bool    is_fuel_ignition;

    /// Состояние вентиля МВ6
    bool    state_mv6;

    /// Состояние вентиля ВТН
    bool    state_vtn;

    /// Заданная частота вращения коленчатого вала, об/мин
    double  n_ref;

    /// Максимальный расход топлива, кг/с
    double  Q_max;

    /// Фактический расход топлива, кг/с
    double  Q_fuel;

    /// Текущий крутящий момент, развиваемый дизелем
    double  M_d;

    /// Минимальная чатота прокрутки коленчатого вала при пуске
    double  omega_min;

    /// Время прокрутки до пуска
    double  start_time;

    /// Таймер, измеряющий время прокрутки до воспламенения топлива
    Timer   *timer;

    /// Давление топлива перед ТНВД
    double  fuel_pressure;

    double  delta_omega;

    std::array<double, NUM_COEFFS>  K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

private:

    void slotFuelIgnition();
};

#endif // DISEL_H
