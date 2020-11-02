#ifndef     FUEL_TANK_H
#define     FUEL_TANK_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FuelTank : public Device
{
public:

    FuelTank(QObject *parent = Q_NULLPTR);

    ~FuelTank();

    /// Получить текущую массу топлива в баке
    double getFuelMass() const { return getY(0); }

    /// Получить текущий уровень топлива в баке
    double getFuelLevel() const {return getY(0) / fuel_capacity; }

    /// Задать емкость топливного бака, кг
    void setCapacity(double fuel_capacity) { this->fuel_capacity = fuel_capacity; }

    /// Задать начальный уровень топлива
    void setFuelLevel(double fuel_level) { setY(0, fuel_capacity * cut(fuel_level, 0.0, 1.0)); }

    /// Задать расход топлива из бака
    void setFuelConsumption(double fuel_consumption) { this->fuel_consumption = fuel_consumption; }

private:

    /// Емкость бака, кг
    double fuel_capacity;

    /// Текущий расход из бака, кг/с
    double fuel_consumption;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // FUEL_TANK_H
