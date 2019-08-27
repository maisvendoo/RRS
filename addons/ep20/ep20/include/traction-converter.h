#ifndef TRACTION_CONVERTER_H
#define TRACTION_CONVERTER_H

#include    "device.h"

class TractionConverter : public Device
{

public:

    TractionConverter(QObject *parent = Q_NULLPTR);

    ~TractionConverter();

    void setUt(double Ut, size_t i);

    void setUdcIn(double Udc_in);

    double getU4();

private:

    /// Входной параметр
    std::array<double, 2> Ut;
    double Udc_in;


    std::array<double, 2> U4QS;
    double K4QS;

    std::array<double, 2> Udc;


    /// Выходной параметр
    double U4;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

};

#endif // TRACTIONCONVERTER_H
