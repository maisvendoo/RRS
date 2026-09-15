#ifndef     PNEUMO_REDUCER_PANEL_H
#define     PNEUMO_REDUCER_PANEL_H

#include    "device.h"
#include    "pneumo-reducer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PneumoReducerPanel : public Device
{
public:

    PneumoReducerPanel(QObject *parent = Q_NULLPTR);

    ~PneumoReducerPanel();

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Поток в резервуар, управляемый редуктором КР1
    double getOutputFlow1() const
    {
        return Q1;
    }

    /// Поток в резервуар, управляемый редуктором КР5
    double getOutputFlow5() const
    {
        return Q5;
    }

    /// Задать давление от резервуара, управляемого редуктором КР1
    void setPressure1(double p_out1);

    /// Задать давление от резервуара, управляемого редуктором КР5
    void setPressure5(double p_out5);

    void step(double t, double dt);

private:

    /// Уставка редуктора KP1
    double p_kp1;

    /// Уставка редуктора KP5
    double p_kp5;

    /// Поток в питательную магистрали
    double QFL;

    /// Расход из редуктора KP1
    double Q1;

    /// Расход из редуктора KP5
    double Q5;

    /// Редуктор питания клапана замещения Y4
    PneumoReducer *kp1;

    /// Редуктор питания клапана ЭТ Y5
    PneumoReducer *kp5;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_REDUCER_PANEL_H
