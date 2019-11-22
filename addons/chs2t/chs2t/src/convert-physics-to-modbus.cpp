#include "convert-physics-to-modbus.h"


PhysToModbus::PhysToModbus()
{

}

PhysToModbus::~PhysToModbus()
{

}

void PhysToModbus::load_config(CfgReader& cfg)
{
    double phys;
    double modbus;

    for (int i = 0; i < 7; i++)
    {
        cfg.getDouble(QString::number(i), "Phys", physValue);
        cfg.getDouble(QString::number(i), "Modbus", modbus);
        modbusValues.insert(physValue, modbus);
    }
}

double PhysToModbus::calculateModbus()
{
//    for (auto i = modbusValues.begin(); i != modbusValues.end(); i++)
//    {
//        if (physValue >= i.key())
//        {
//            physValue = i.key();
//        }
//    }

    return 0;
}

