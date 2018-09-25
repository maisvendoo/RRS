#include    "test-loco.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TestLoco::TestLoco() : Vehicle()
  , traction_level(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TestLoco::~TestLoco()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::step(double t, double dt)
{
    (void) t;
    double traction_step = 0.0;

    if (keys[Qt::Key_A])
        traction_step = 0.01;

    if (keys[Qt::Key_D])
        traction_step = -0.01;

    traction_level +=  traction_step * dt;

    traction_level = Physics::cut(traction_level, 0.0, 1.0);

    for (size_t i = 0; i < Q_a.size(); i++)
    {
        double torque = traction_level * traction_char(velocity) * wheel_diameter / num_axis / 2.0;
        Q_a[i] = torque;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TestLoco::traction_char(double v)
{
    double max_traction = 600e3;    

    double vn = 81.0 / Physics::kmh;

    if (abs(v) <= vn)
        return max_traction;
    else
        return max_traction * vn / v;
}

GET_VEHICLE(TestLoco)
