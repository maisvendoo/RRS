#ifndef     PNEUMO_SPLITTER_H
#define     PNEUMO_SPLITTER_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoSplitter : public BrakeDevice
{
public:

    PneumoSplitter(double work_volume = 1e-3, QObject *parent = Q_NULLPTR);

    ~PneumoSplitter();

    // Работа с давлением в рабочей камере пневмосоединения напрямую
    /// Задать давление входящей магистрали
    void setInputPressure(double value);

    /// Суммарный поток из исходящих магистралей во входящую
    double getSumFlow() const;

    // Работа с камерой пневмосоединения как с отдельным объёмом
    /// Задать поток из входящей магистрали
    void setInputFlow(double value);

    /// Давление в рабочей камере пневмосоединения
    double getInputPressure() const;

    /// Задать давление от первой исходящей магистрали
    void setPipePressure1(double value);

    /// Поток в первую исходящую магистраль
    double getPipeFlow1() const;

    /// Задать давление от второй исходящей магистрали
    void setPipePressure2(double value);

    /// Поток во вторую исходящую магистраль
    double getPipeFlow2() const;

private:

    /// Объём рабочей камеры пневмосоединения
    double V0;
    /// Признак установки давления в рабочей камере напрямую
    bool is_set_pressure;

    double pIN;
    double p1;
    double p2;

    double QIN;
    double Q1;
    double Q2;

    enum
    {
        NUM_COEFFS = 4
    };

    std::array<double, NUM_COEFFS> K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_SPLITTER_H
