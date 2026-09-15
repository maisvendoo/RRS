#ifndef     FAST_SWITCH_H
#define     FAST_SWITCH_H

#include    "relay.h"
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FastSwitch : public Relay
{
public:

    FastSwitch(QObject *parent = Q_NULLPTR);

    ~FastSwitch();

    void setControlVoltage(double Uc) { this->Uc = Uc; }

    void setPowerOn(bool is_power_On) { this->is_power_On = is_power_On; }

    void setHold(bool is_Hold) { this->is_Hold = is_Hold; }

    void setCurrent(double Id) { this->Id = Id; }

private:

    double Uc;

    bool is_power_On;

    bool is_Hold;

    /// Ток через главные контакты
    double Id;

    /// Ток уставки через главные контакты
    double Id_max;

    /// Триггер, для описания состояния включающей катушки
    Trigger power_on_coil;

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);
};

#endif // FAST_SWITCH_H
