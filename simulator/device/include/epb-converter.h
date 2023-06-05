//------------------------------------------------------------------------------
//
//      Статический преобразователь питания ЭПТ
//
//------------------------------------------------------------------------------
#ifndef     EPB_CONVERTER_H
#define     EPB_CONVERTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT EPBConverter  : public Device
{
public:

    EPBConverter(QObject *parent = Q_NULLPTR);

    ~EPBConverter();

    /// Задать входное напряжение от сети локомотива
    void setInputVoltage(double value);

    /// Задать ток, потребляемый электропневматическими тормозами
    void setOutputCurrent(double value);

    /// Выходное напряжение для управления электропневматическими тормозами
    double getOutputVoltage();

    /// Частота переменного тока для управления электропневматическими тормозами
    double getOutputFrequency();

    virtual void step(double t, double dt);

private:

    /// Напряжение питания перобразователя (аккумуляторная батарея)
    double U_bat;

    /// Внутреннее сопротивление
    double r;

    /// Коэффициент передачи статического преобразователя
    double ks;

    /// Выходная частота переменного тока в блок управления ЭПТ
    double f;

    /// Выходное напряжение в блок управления ЭПТ
    double U_out;

    /// Выходной ток, потребляемый приборами торможения
    double I_out;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EPB_CONVERTER_H
