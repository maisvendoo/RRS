#ifndef     COUPLING_OPERATING_ROD_H
#define     COUPLING_OPERATING_ROD_H

#include    "device.h"

//------------------------------------------------------------------------------
// Расцепной рычаг для автосцепки
//------------------------------------------------------------------------------
class DEVICE_EXPORT OperatingRod : public Device
{
public:

    /// Конструктор
    OperatingRod(int key_code = 0, QObject *parent = Q_NULLPTR);

    /// Деструктор
    virtual ~OperatingRod();

    /// Задать управляющую клавишу
    void setKeyCode(int key_code);

    /// Задать усилия в сцепке, Н
    void setCouplingForce(double force);

    /// Получить положение расцепного рычага:
    /// от 1.0 (нормальное), 0.0 (натянутая цепочка) до -1.0 (расцепляющее)
    double getOperatingState() const;

    /// /TODO/ Признак фиксации расцепного рычага в расцепляющем положении
    bool isFixedUncoupled() const;

protected:

    /// Код управляющей клавиши
    int keyCode;

    /// Целевое положение рычага: 1.0 - нормальное; -1.0 - расцепляющее
    double ref_operating_state;

    /// /TODO/ Признак фиксации расцепного рычага в расцепляющем положении
    bool is_fixed_uncoupling;

    /// Усилие в сцепке
    double coupling_force;

    /// Максимальное усилие в сцепке, при котором возможно управление сцепным устройством, Н
    double max_operating_force;

    /// Время движения расцепного рычага между положениями, с
    double motion_time;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);

    virtual void stepKeysControl(double t, double dt);
};

#endif // COUPLING_OPERATING_ROD_H
