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
    OperatingRod(QObject *parent = nullptr);

    /// Деструктор
    virtual ~OperatingRod();

    /// Задать управляющую клавишу
    void setKeySymbol(std::uint16_t key_symbol);

    /// Задать усилия в сцепке, Н
    void setCouplingForce(double force);

    /// Получить положение расцепного рычага:
    /// от 1.0 (нормальное), 0.0 (натянутая цепочка) до -1.0 (расцепляющее)
    double getOperatingState() const;

    /// Признак фиксации расцепного рычага в расцепляющем положении
    bool isFixedUncoupled() const;

protected:

    /// Код управляющей клавиши
    std::uint16_t key_symbol_operate = KEY_Undefined;

    /// Предыдущее состояние управляющей клавиши
    bool prev_key = false;

    /// Признак фиксации расцепного рычага в расцепляющем положении
    bool is_fixed_uncoupling = false;

    /// Целевое положение рычага: 1.0 - нормальное; -1.0 - расцепляющее
    double ref_operating_state = 1.0;

    /// Усилие в сцепке
    double coupling_force = 0.0;

    /// Максимальное усилие в сцепке, при котором возможно управление сцепным устройством, Н
    double max_operating_force = Physics::ZERO;

    /// Время движения расцепного рычага между положениями, с
    double motion_time = 0.1;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void stepKeysControl(double t, double dt);

    /// Загрузка параметров из конфигурационного файла
    void load_config(CfgReader &cfg);
};

#endif // COUPLING_OPERATING_ROD_H
