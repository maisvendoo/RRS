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
#include    "brake-crane.h"

class TestLoco : public Vehicle
{
public:

    TestLoco();
    ~TestLoco();

private:

    double traction_level;

    bool inc_loc;
    bool dec_loc;

    QString brake_crane_module;
    BrakeCrane  *brake_crane;

    void step(double t, double dt);

    double traction_char(double V);

    void initialization();

    void loadConfig(QString cfg_path);
};

#endif // TESTLOCO_H
