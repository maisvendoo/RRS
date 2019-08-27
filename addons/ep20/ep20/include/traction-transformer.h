#ifndef TRACTION_TRANSFORMER_H
#define TRACTION_TRANSFORMER_H

#include "device.h"

class TractionTransformer : public Device
{

public:

    ///
    TractionTransformer(QObject *parent = Q_NULLPTR);

    ~TractionTransformer();

    void setU1(double U1);

    double getTractionVoltage(size_t i);

    double getVoltageHeatingCoil();

private:

    /// Напряжение с выхода ГВ
    double U1;

    /// Коэффициент тяговой обмотки
    double traction_coefficient;

    /// Коэффициент обмотки отопления
    double heating_coil_coefficient;

    /// Катушка обмотки отопления
    double winding_coils_heating;

    /// Катушки тяговой обмотки
    std::array<double, 6> traction_winding_coils;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

};

#endif // TRACTIONTRANSFORMER_H
