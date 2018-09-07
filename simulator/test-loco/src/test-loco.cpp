#include    "test-loco.h"
#include    "physics.h"

TestLoco::TestLoco() : Vehicle()
  , traction_force(0.0)
{

}

TestLoco::~TestLoco()
{

}

void TestLoco::step(double t, double dt)
{
    double max_traction_force = 600e5;
    double k = 1e4;

    traction_force += k * dt;

    traction_force = Physics::cut(traction_force, 0.0, max_traction_force);

    for (size_t i = 0; i < Q_a.size(); i++)
    {
        double torque = 2 * traction_force / wheel_diameter / num_axis;
        Q_a[i] = torque;
    }
}

GET_VEHICLE(TestLoco)
