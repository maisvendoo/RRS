#ifndef     SANDING_SYSTEM_H
#define     SANDING_SYSTEM_H

#include    "device.h"

//------------------------------------------------------------------------------
// Система подачи песка на рельсы под колесо
//------------------------------------------------------------------------------
class DEVICE_EXPORT SandingSystem : public Device
{
public:

    SandingSystem(QObject *parent = Q_NULLPTR);

    virtual ~SandingSystem();

    /// Задать состояние подачи песка
    void setSandDeliveryOn(bool state);

    /// Состояние подачи песка
    bool isSandDelivery() const;

    /// Расход подачи песка, кг/мин
    double getSandFlow() const;

    /// Пересчёт коэффициента трения колесо-рельс
    double getWheelRailFrictionCoeff(double current_coeff) const;

    /// Задать вместимость бункера для песка, кг
    void setSandMassMax(double value);

    /// Задать количество песка в бункере, кг
    void setSandMass(double value);

    /// Задать относительный уровень песка в бункере, 0.0 - 1.0
    void setSandLevel(double level);

    /// Количество песка в бункере, кг
    double getSandMass() const;

    /// Относительный уровень песка в бункере, 0.0 - 1.0
    double getSandLevel() const;

    /// Задать давление от питательной магистрали, МПа
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

protected:

    /// Состояние подачи песка
    bool is_sand;

    /// Максимальная вместимость бункера для песка, кг
    double sand_mass_max;

    /// Номинальный расход песка, кг/мин
    double sand_flow_nom;

    /// Расход песка, кг/мин
    double sand_flow;

    /// Давление питательной магистрали, МПа
    double pFL;

    /// Поток в питательную магистраль
    double QFL;

    /// Номинальное давление для пневмоподачи песка, МПа
    double p_nom;

    /// Коэффициент потока - расхода воздуха для подачи песка
    double k_air;

    /// Коэффициент изменения трения колесо-рельс
    double k_friction;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    virtual void stepKeysControl(double t, double dt);
};

#endif // SANDING_SYSTEM_H
