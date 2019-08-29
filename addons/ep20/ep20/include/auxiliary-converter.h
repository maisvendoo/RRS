#ifndef AUXILIARY_CONVERTER_H
#define AUXILIARY_CONVERTER_H

#include    "device.h"

class AuxiliaryConverter : public Device
{

public:

    AuxiliaryConverter(QObject *parent = Q_NULLPTR);

    ~AuxiliaryConverter();

    void setU4(double U4);

    double getU1();

    double getF();

    double getU2();

private:

    /// Входные параметра

    double U4;

    double Fref;

    double Uref;

    double Koef;

    ///Выходные параметры

    double U1;

    double F;

    double U2;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);
};

#endif // AUXILIARYCONVERTER_H
