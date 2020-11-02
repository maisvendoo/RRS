#ifndef     EPT_PASS_CONTROL_H
#define     EPT_PASS_CONTROL_H

#include    "device.h"

class DEVICE_EXPORT EPTPassControl : public Device
{
public:

    EPTPassControl(QObject *parent = Q_NULLPTR);

    ~EPTPassControl();

    void setHoldState(bool is_hold) { this->is_hold = is_hold; }

    void setBrakeState(bool is_brake) { this->is_brake = is_brake; }

    void setU(double U) { this->U = U; }

    double getControlSignal() const { return control_signal; }

    float stateReleaseLamp() const { return lampRelease; }

    float stateHoldLamp() const { return lampHold; }

    float stateBrakeLamp() const { return lampBrake; }

private:

    /// Флаг перекрыши
    bool is_hold;

    /// Флаг торможения
    bool is_brake;

    /// Напряжение питания линии управления
    double U;

    /// Сигнал управления вентилями ЭПТ
    double control_signal;

    /// Состояние лампы "О"
    float lampRelease;

    /// Состояние лампы "П"
    float lampHold;

    /// Состояние лампы "т"
    float lampBrake;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EPT_PASS_CONTROL_H
