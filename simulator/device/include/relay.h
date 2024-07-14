#ifndef     RELAY_H
#define     RELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
// Реле
//------------------------------------------------------------------------------
class DEVICE_EXPORT Relay : public Device
{
public:

    Relay(size_t num_contacts = 1, QObject *parent = Q_NULLPTR);

    ~Relay();

    /// Задать начальное состояние контакта
    void setInitContactState(size_t number, bool state);

    /// Задать напряжение, подаваемое на обмотку реле, В
    virtual void setVoltage(double U);

    /// Состояние контакта
    virtual bool getContactState(size_t number = 0) const;

    /// Ток, потребляемый реле, А
    virtual double getCurrent() const;

    /// Состояние звука включения реле
    sound_state_t getSoundOn() const;

    /// Состояние звука выключения реле
    sound_state_t getSoundOff() const;

protected:

    /// Предыдущее состояние якоря реле
    bool    ancor_state_prev;

    /// Напряжение, подаваемое на обмотку
    double  U;

    /// Номинальное рабочее напряжение
    double  U_nom;

    /// Активное сопротивление якоря реле
    double  R;

    /// Постоянная времени обмотки
    double  T;

    /// Максимальный уровень отключения (0.0 - 1.0)
    double  level_off;

    /// Минимальный уровень включения (0.0 - 1.0)
    double  level_on;

    /// Гистерезис минимального тока включения - максимального тока отключения
    Hysteresis *hysteresis;

    std::vector<bool>    contact;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // RELAY_H
