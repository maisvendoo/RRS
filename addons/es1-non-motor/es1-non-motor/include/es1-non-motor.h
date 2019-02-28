#ifndef     SAPSAN_MOTOR_H
#define     SAPSAN_MOTOR_H

#include    "vehicle.h"
#include    "controls.h"
#include    "brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_BRAKE_POS = 7
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ES1NonMotor : public Vehicle
{
public:

    ES1NonMotor();

    ~ES1NonMotor();

private:

    int     brake_pos;
    double  brake_step;
    double  pBC_max;

    BrakeMech *brake_mech;

    QString     brake_mech_module;
    QString     brake_mech_config;

    IncBrakePos incBrakePos;

    DecBrakePos deccBrakePos;

    void initialization();

    void keyProcess();

    void step(double t, double dt);

    double traction_char(double v);
};

#endif // SAPSAN_MOTOR_H
