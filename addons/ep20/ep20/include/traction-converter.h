#ifndef TRACTION_CONVERTER_H
#define TRACTION_CONVERTER_H

#include    "device.h"

/// Класс Тягового Преобразователя
class TractionConverter : public Device
{

public:

    /// Конструктор
    TractionConverter(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~TractionConverter();

    /// Установить напряжение с тяговых обмоток
    void setUt(double Ut, size_t i);

    /// Установить напряжение постоянного тока
    void setUdcIn(double Udc_in);

    /// Получить напряжение на преобразователь собственных нужд
    double getU4(size_t i);

private:

    /// Напряжение с тяговых обмоток
    std::array<double, 2> Ut;

    /// Напряжение постоянного тока
    double Udc_in;

    /// Напряжение с выхода 4QS-преобразователя
    std::array<double, 2> U4QS;

    /// Коэффициент 4QS-преобразователя
    double K4QS;

    /// Напряжение звена постоянного тока
    std::array<double, 2> Udc;

    /// Напряжение преобразователя собственных нужд
    std::array<double, 2> U4;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

};

#endif // TRACTIONCONVERTER_H
