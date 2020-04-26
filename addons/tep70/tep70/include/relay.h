#ifndef     RELAY_H
#define     RELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Relay : public Device
{
public:

    Relay(size_t num_contacts = 1, QObject *parent = Q_NULLPTR);

    ~Relay();


    virtual void setInitContactState(size_t number, bool state);

    virtual void setVoltage(double U);

    virtual bool getContactState(size_t number) const;

    virtual double getCurrent() const;

    void setSoundName(QString soundName) { this->soundName = soundName; }

protected:

    /// Текущее состояние якоря реле
    bool    ancor_state_cur;

    /// Предыдущее состояние якоря реле
    bool    ancor_state_prev;

    /// Активное сопротивление якоря реле
    double  R;

    /// Напряжение, подаваемое на обмотку
    double  U;

    /// Постоянная времени обмотки
    double  T;

    /// Минимальный ток включения
    double  I_on;

    /// Максимальный ток отключения
    double  I_off;

    /// Значение тока на предыдущем шаге
    double  I_prev;

    /// Имя звука при включении и выключении
    QString soundName;

    std::vector<bool>    contact;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);

    void change_contacts_state();

    bool histeresis(double I, double Imin, double Imax);
};

#endif // RELAY_H
