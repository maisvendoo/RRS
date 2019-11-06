#ifndef     EPT_CONTROL_LINE_H
#define     EPT_CONTROL_LINE_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT EPTControlLine : public Device
{
public:

    EPTControlLine(QObject *parent = Q_NULLPTR);

    ~EPTControlLine();

    void setInputVoltage(double U_in) { this->U_in = U_in; }

    void setOutputCurrent(double I_out) { this->I_out = I_out; }

    void setValvesNumber(size_t num);

    double getI_in() const { return getY(0); }

    double getU_out() const { return U_out; }

    void setValveState(size_t i, double state) { u[i] = state; }

protected:

    /// Входное напряжение
    double U_in;
    /// Выходное напряжение
    double U_out;

    /// Индуктивность линии
    double L;

    /// Активное сопротивление линии
    double r;

    /// Выходной ток
    double I_out;

    /// Эквивалентная проводимость всех вентилей
    double G;

    /// Состояние вентилей на линии
    std::vector<double> u;

    /// Активное сопротивление вентилей
    std::vector<double> R;

private:

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EPTCONTROLLINE_H
