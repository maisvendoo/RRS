#ifndef     PNEUMO_RELAY_H
#define     PNEUMO_RELAY_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoRelay : public BrakeDevice
{
public:

    PneumoRelay(double work_volume = 1e-3, QObject *parent = Q_NULLPTR);

    virtual ~PneumoRelay();

    // Работа с давлением в управляющей камере напрямую
    /// Задать управляющее давление в повторительном пневмореле
    void setControlPressure(double value);

    // Работа с управляющей камерой как с фиктивным объёмом
    /// Задать управляющий поток в повторительное пневмореле
    void setControlFlow(double value);

    /// Давление в управляющей камере повторительного пневмореле
    double getControlPressure() const;

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от управляемой магистрали
    void setPipePressure(double value);

    /// Поток в управляемую магистраль
    double getPipeFlow() const;

protected:

    /// Объём управляющей камеры
    double V0;
    /// Признак установки давления в рабочей камере напрямую
    bool is_set_pressure;

    double pCONTROL;
    double pFL;
    double pPIPE;

    double QCONTROL;
    double QFL;
    double QPIPE;

    /// Точность поддержания давления, МПа
    double eps;
    /// Коэффициент к положению диафрагмы управлющей камеры
    double A1;
    /// Коэффициент к наполнению управляемой магистрали из питательной
    double K1;
    /// Коэффициент к разрядке управляемой магистрали в атмосферу
    double K2;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    /// Load configuration
    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_RELAY_H
