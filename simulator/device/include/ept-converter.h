//------------------------------------------------------------------------------
//
//      Статический преобразователь питания ЭПТ
//
//
//------------------------------------------------------------------------------
#ifndef     EPT_CONVERTER_H
#define     EPT_CONVERTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT EPTConverter  : public Device
{
public:

    EPTConverter(QObject *parent = Q_NULLPTR);

    ~EPTConverter();

    void setU_bat(double U_bat) { this->U_bat = U_bat; }

    void setI_out(double I_out) { this->I_out = I_out; }

    double getU_out() const { return U_out; }

private:

    /// Напряжение питания перобразователя (аккумуляторная батарея)
    double U_bat;

    /// Внутреннее сопротивление
    double r;

    /// Коэффициент передачи статического преобразователя
    double ks;

    /// Выходное напряжение на линию управления ЭПТ
    double U_out;

    /// Выходной ток, потребляемый приборами торможения
    double I_out;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EPT_CONVERTER_H
