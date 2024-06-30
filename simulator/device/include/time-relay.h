#ifndef     TIME_RELAY_H
#define     TIME_RELAY_H

#include    "relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TimeRelay : public Relay
{
public:

    TimeRelay(size_t num_contacts = 1,
              double delay_on = 0.0,
              double delay_off = 0.0,
              QObject *parent = Q_NULLPTR);

    ~TimeRelay();

    /// Задать задержку срабатывания при включении, с
    void setTimeoutOn(double timeout);

    /// Задать задержку при отключении, с
    void setTimeoutOff(double timeout);

    /// Задать напряжение питания, В
    void setControlVoltage(double Uc);

    /// Потребляемый ток, А
    double getCurrent() const override;

    void step(double t, double dt) override;

private:

    /// Управляющее напряжение
    double      Uc;

    /// Внутреннее сопротивление управляющих цепей
    double      Rc;

    /// Порог срабатывания
    double      start_level;

    /// Флаг срабатывания
    bool        is_on;

    Timer       *timer_on;
    Timer       *timer_off;

    void setVoltage(double U) override;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

private slots:

    void slotTimeoutProcessOn();
    void slotTimeoutProcessOff();
};

#endif // TIME_RELAY_H
