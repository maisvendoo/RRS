#ifndef CONVERTPHYSICSTOMODBUS_H
#define CONVERTPHYSICSTOMODBUS_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PhysToModbus
{
public:
    PhysToModbus();

    ~PhysToModbus();

    void setPhysValue(double value) { physValue = value; }

    double getModbus() { return calculateModbus(); }

private:
    QMap <double, double> modbusValues;

    void load_config(CfgReader &cfg);

    double physValue;

    double calculateModbus();
};

#endif // CONVERTPHYSICSTOMODBUS_H
