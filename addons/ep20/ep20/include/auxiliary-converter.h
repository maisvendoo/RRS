#ifndef AUXILIARY_CONVERTER_H
#define AUXILIARY_CONVERTER_H

#include    "device.h"

/// Класс Преобразователя Собственных Нужд
class AuxiliaryConverter : public Device
{

public:

    /// Конструктор
    AuxiliaryConverter(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~AuxiliaryConverter();

    /// Установить напряжение преобразователя собственных нужд
    void setU4(double U4);

    /// Получить напряжение
    double getU1();

    /// Получить частоту
    double getF();

    /// получить напряжение (380 В)
    double getU2();

private:

    /// Напряжение преобразователя собственных нужд
    double U4;

    /// Частота
    double Fref;

    /// Напряжение
    double Uref;

    /// Коэффициент напряжения
    double Koef;

    /// Напряжение на выход
    double U1;

    /// Частота на выход
    double F;

    /// Напряжение 380 на выход
    double U2;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);
};

#endif // AUXILIARYCONVERTER_H
