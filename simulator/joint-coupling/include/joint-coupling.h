#ifndef     JOINT_COUPLING_H
#define     JOINT_COUPLING_H

#include    "device-joint.h"

#include    "coupling-data.h"

#include    <utility>
#include    <vector>

enum
{
    NUM_CONNECTORS = 2,
    FWD = 0,
    BWD = 1
};

//------------------------------------------------------------------------------
// Соединение сцепок
//------------------------------------------------------------------------------
class JointCoupling : public Joint
{
public:

    /// Конструктор
    JointCoupling();
    ~JointCoupling();

    /// Шаг симуляции
    virtual void step(double t, double dt);

    /// Сцепки соединены
    bool isConnected() const;

    /// Сцепка разрушена (перегрузкой)
    bool isBroken() const;

    /// Текущее усилие в сцепке, Н (положительное - растяжение)
    double getForce() const;

    /// Текущая относительная скорость сцепок, м/с
    double getRelVelocity() const;

    /// Повреждение сцепного устройства (0 - целая, 1 - разрушена)
    double getDamage() const;

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

    /// Коэффициент потерь энергии на пластические деформации конструкций
    double fk;

    /// Упругость конструкций после сжатия поглощающих аппаратов
    double  ck;

    /// Максимальное растягивающее усилие (разрушение), Н
    double max_tension;

    /// Максимальное сжимающее усилие (разрушение), Н
    double max_compression;

    /// Порог начала накопления повреждений, Н
    double damage_force;

    /// Работа повреждающей силы до разрушения, Дж
    double break_energy;

    /// Табличная характеристика растяжения (деформация, м -> сила, Н)
    std::vector<std::pair<double, double>> tension_curve;

    /// Табличная характеристика сжатия (|деформация| -> |сила|, Н)
    std::vector<std::pair<double, double>> compression_curve;

    /// Вязкое демпфирование при растяжении, Н*с/м
    double damping_tension;

    /// Вязкое демпфирование при сжатии, Н*с/м
    double damping_compression;

    /// Тип сцепного устройства: 0 - совместимая, 1 - винтовая
    /// (ползучая скорость сцепления), 3 - несовместимая (не сцепляется)
    int coupler_type = 0;

    /// Максимальная скорость сцепления, м/с
    double max_coupling_speed = 1.5;

    /// Повреждение 0..1
    double damage;

    /// Сцепка разрушена
    bool broken;

    /// Текущее усилие (для диагностики)
    double cur_force;

    /// Текущая относительная скорость сцепок (для диагностики)
    double cur_rel_velocity;

    /// Расчёт усилия в сцепке
    virtual double calc_force(double ds, double dv);

    /// Накопление повреждений и проверка разрушения
    void updateDamage(double force, double dv, double dt);

    /// Загрузка параметров из конфига
    virtual void load_config(CfgReader &cfg);
/*
    QString msg;
    Registrator *reg;
*/
};

#endif // JOINT_COUPLING_H
