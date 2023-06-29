#ifndef     PNEUMO_REGUCER_H
#define     PNEUMO_REGUCER_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoReducer : public BrakeDevice
{
public:

    PneumoReducer(double ref_pressure = 0.5, QObject *parent = Q_NULLPTR);

    ~PneumoReducer();

    /// Задать уставку давления на выходе редуктора
    void setRefPressure(double value);

    /// Задать давление на входе редуктора
    void setInputPressure(double value);

    /// Поток на входе в редуктор
    double getInputFlow() const;

    // Работа с давлением в управляемой камере редуктора напрямую
    /// Задать давление на выходе редуктора
    void setOutPressure(double value);

    /// Поток из выхода редуктора
    double getOutFlow() const;

    // Работа с управляемой камерой редуктора как с фиктивным объёмом
    /// Задать поток из выхода пневморедуктора
    void setOutFlow(double value);

    /// Давление на выходе пневморедуктора
    double getOutPressure() const;

private:

    /// Объем управляемой камеры на выходе
    double V0;
    /// Признак установки давления в управляемой камере напрямую
    bool is_set_pressure;

    /// Уставка давления на выходе редуктора
    double pREF;

    /// Давление на входе редуктора
    double pIN;

    /// Расход в управляемую камеру
    double QIN;

    /// Давление в управляемой камере редуктора
    double pOUT;

    /// Расход из управляемой камеры редуктора
    double QOUT;

    /// Коэффициент к условному клапану наполнения управляемой камеры
    double kv;

    /// Коэффициент к потоку наполнения управляемой камеры
    double Kflow;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_REGUCER_H
