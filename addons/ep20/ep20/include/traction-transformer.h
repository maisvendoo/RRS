#ifndef TRACTION_TRANSFORMER_H
#define TRACTION_TRANSFORMER_H

#include "device.h"


/// Класс Тягового Трансформатора
class TractionTransformer : public Device
{

public:

    /// Конструктор
    TractionTransformer(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~TractionTransformer();

    /// Установить напряжение с выхода ГВ
    void setU1(double U1);

    /// Получить напряжение тяговой обмотки
    double getTractionVoltage(size_t i);

    /// Получить напряжение обмотки отопления
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
