#ifndef     SANDING_SYSTEM_H
#define     SANDING_SYSTEM_H

#include    "device.h"

//------------------------------------------------------------------------------
// Система подачи песка на рельсы под колесо
//------------------------------------------------------------------------------
class DEVICE_EXPORT SandingSystem : public Device
{
public:

    SandingSystem(QObject *parent = nullptr);

    virtual ~SandingSystem();

    /// Задать управляющую клавишу
    void setKeySymbol(std::uint16_t key_symbol);

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

    /// Состояние звука работы песочницы
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал состояния звука работы песочницы
    virtual float getSoundSignal(size_t idx = 0) const;

protected:

    /// Код управляющей клавиши
    std::uint16_t key_symbol_operate = KEY_Undefined;

    /// Состояние подачи песка
    bool is_sand = false;

    /// Максимальная вместимость бункера для песка, кг
    double sand_mass_max = 2000.0;

    /// Номинальный расход песка, кг/мин
    double sand_flow_nom = 2.0;

    /// Расход песка, кг/мин
    double sand_flow = 2.0;

    /// Давление питательной магистрали, МПа
    double pFL = 0.0;

    /// Поток в питательную магистраль
    double QFL = 0.0;

    /// Номинальное давление для пневмоподачи песка, МПа
    double p_nom = 0.9;

    /// Коэффициент потока - расхода воздуха для подачи песка
    double k_air = 5.0e-4;

    /// Коэффициент изменения трения колесо-рельс
    double k_friction = 1.3;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    virtual void stepKeysControl(double t, double dt);
};

#endif // SANDING_SYSTEM_H
