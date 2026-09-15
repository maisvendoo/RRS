#ifndef     PKD15_H
#define     PKD15_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BrakeSwitcher : public Device
{
public:

    BrakeSwitcher(size_t num_contacts = 1, QObject *parent = Q_NULLPTR);

    ~BrakeSwitcher();

    void setPressure(double p) { this->p = p; }

    void setTracValveState(double trac_valve_state)
    {
        this->trac_valve_state = trac_valve_state;
    }

    void setBrakeValveState(double brake_valve_state)
    {
        this->brake_valve_state = brake_valve_state;
    }

    void setInitContactState(size_t number, bool state)
    {
        if (number < contact.size())
            contact[number] = state;
    }

    bool getContactState(size_t number) const
    {
        if (number < contact.size())
            return contact[number];

        return false;
    }

    int getMotorMode() const
    {
        return mode;
    }

private:

    /// Время переключения с одного положения на другое
    double tau;

    /// Состояние вентиля "тяга"
    bool trac_valve_state;

    /// Состояние вентиля "торможение"
    bool brake_valve_state;

    /// Текущее давление в пневматической магистрали управления
    double p;

    /// Номинальное рабочее давление
    double p_nom;

    /// Условная скорость переключения номинальна
    double omega_nom;

    /// Условная скорость переключения фактическая
    double omega;

    int dir;

    int dir_old;

    int mode;

    std::vector<bool>    contact;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void change_contacts_state();
};

#endif // PKD15_H
