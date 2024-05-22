#ifndef     JOINT_COUPLING_SA3_H
#define     JOINT_COUPLING_SA3_H

#include    "device-joint.h"

#include    "coupling-data.h"

enum
{
    NUM_CONNECTORS = 2,
    FWD = 0,
    BWD = 1
};

//------------------------------------------------------------------------------
// Соединение сцепок СА-3
//------------------------------------------------------------------------------
class JointCouplingSA3 : public Joint
{
public:

    /// Конструктор
    JointCouplingSA3();

    /// Деструктор
    ~JointCouplingSA3();

    /// Шаг симуляции
    virtual void step(double t, double dt);

private:

    /// Состояние сцепок
    bool is_connected;

    /// Зазор в сцепном устройстве
    double delta;

    /// Коэффициент трения поглощающего аппарата
    double f;

    /// Упругость поглощающих аппаратов
    double c;

    /// Максимальное сжатие поглощающих аппаратов
    double lambda;

    /// Упругость конструкций после сжатия поглощающих аппаратов
    double  ck;

    /// Усилие до начала сжатия поглощающего аппарата
    double  T0;

    /// Деформация до начала сжатия поглощающего аппарата
    double  dx_t0;

    /// Расчёт усилия в сцепке
    virtual double calc_force(double ds, double dv);

    /// Загрузка параметров из конфига
    virtual void load_config(CfgReader &cfg);
/*
    QString msg;
    Registrator *reg;
*/
};

#endif // JOINT_COUPLING_SA3_H
