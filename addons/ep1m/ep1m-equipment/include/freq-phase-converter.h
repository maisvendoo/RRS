#ifndef     FREQ_PHASE_CONVERTER_H
#define     FREQ_PHASE_CONVERTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//  Преобразователь частоты и числа фаз (ПЧФ)
//------------------------------------------------------------------------------
class FreqPhaseConverter : public Device
{
public:

    FreqPhaseConverter(QObject *parent = Q_NULLPTR);

    ~FreqPhaseConverter();

    double getFrequencyLow() const { return freq_low; }

    double getFrequencyNorm() const { return freq_norm; }

    void setInputVoltage(double U_in) { this->U_in = U_in; }

    double getOutputVoltage() const { return U_out; }

private:

    /// Входное напряжение
    double U_in;

    /// Выходное напряжение
    double U_out;

    /// Пониженная частота питания вспомогательных машин
    const double freq_low = 16.67;

    /// Нормальная частота питания вспомогательных машин
    const double freq_norm = 50.0;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // FREQ_PHASE_CONVERTER_H
