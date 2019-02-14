#ifndef     SAPSAN_MOTOR_H
#define     SAPSAN_MOTOR_H

#include    "vehicle.h"

class SapsanMotor : public Vehicle
{
public:

    SapsanMotor();

    ~SapsanMotor();

private:

    double  traction_level;
    bool    inc_loc;
    bool    dec_loc;

    void keyProcess();

    void step(double t, double dt);

    double traction_char(double v);
};

#endif // SAPSAN_MOTOR_H
