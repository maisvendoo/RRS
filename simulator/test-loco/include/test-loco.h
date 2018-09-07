//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     TEST_LOCO_H
#define     TEST_LOCO_H

#include    "vehicle.h"

class TestLoco : public Vehicle
{
public:

    TestLoco();
    ~TestLoco();

private:

    double traction_force;

    void step(double t, double dt);
};

#endif // TESTLOCO_H
