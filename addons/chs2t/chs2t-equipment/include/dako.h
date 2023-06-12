#ifndef DACO_H
#define DACO_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    /// Давление в нижней камере ДАКО, на скорости более 80 км/ч
    /// повторяет давление от КВТ и клапана экстренного торможения
    DACO_BOTTOM_CAMERA = 0,

    /// Давление в средней камере ДАКО, при отключенном реостатном торможении
    /// повторяет давление в управляющем резервуаре (ложном ТЦ) воздухораспределителя
    DACO_MIDDLE_CAMERA = 1
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Dako : public Device
{
public:

    ///Конструктор
    Dako(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Dako();

    /// Задать радиус колёс
    void setWheelRadius(double value);

    /// Задать скорость вращения центробежного регулятора на первой оси
    void setAngularVelocity1(double value);

    /// Задать скорость вращения центробежного регулятора на шестой оси
    void setAngularVelocity6(double value);

    /// Задать ток реостатного торможения
    void setEDTcurrent(double value);

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от магистрали тормозных цилиндров
    void setBCpressure(double value);

    /// Поток в магистраль тормозных цилиндров
    double getBCflow() const;

    /// Задать давление от крана вспомогательного тормоза и клапана экстренного торможения
    void setLocoCranePressure(double value);

    /// Задать давление от воздухораспределителя
    void setAirDistPressure(double value);

    bool isPneumoBrakesRelease() const;

    bool isEDTAllow() const;

private:

    double V0;
    double V1;

    double U1;
    double U6;

    double wheel_r;
    double w1;
    double w1_cur;
    double w6;
    double w6_cur;

    double I;
    double Ia;

    double pFL;
    double pBC;
    double pKVT;
    double pAD;

    double QFL;
    double QBC;

    double A1;
    double A2;

    double K1;
    double K2;
    double K3;
    double K4;
    double K5;
    double K6;

    double k_1;
    double k_2;
    double k_3;
    double k_4;

    bool release_valve_state;
    bool EDT_state;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    /// Загрузка данных из конфигурационных файлов
    void load_config(CfgReader &cfg);
};


#endif // DACO_H
