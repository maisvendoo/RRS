#ifndef     SAPSAN_MOTOR_H
#define     SAPSAN_MOTOR_H

#include    "vehicle.h"
#include    "controls.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ES1Motor : public Vehicle
{
public:

    ES1Motor();

    ~ES1Motor();

private:

    double  traction_level;
    bool    inc_loc;
    bool    dec_loc;

    bool    auto_reg;

    double  vz;

    EnableSpeedReg enSpeedReg;

    IncRefSpeed incRefSpeed;
    DecRefSpeed decRefSpeed;

    void keyProcess();

    void step(double t, double dt);

    double traction_char(double v);
};

#endif // SAPSAN_MOTOR_H
