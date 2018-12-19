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

    double traction_level;

    bool inc_loc;
    bool dec_loc;

    void step(double t, double dt);

    double traction_char(double V);
};

#endif // TESTLOCO_H
