//------------------------------------------------------------------------------
//
//      Vehicle base class
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief  Vehicle base class
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#include    "vehicle.h"

#include    "CfgReader.h"
#include    "physics.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle::Vehicle(QObject *parent) : QObject(parent)
  , idx(0)
  , empty_mass(24000.0)
  , payload_mass(68000.0)
  , payload_coeff(0.0)
  , full_mass(empty_mass + payload_mass * payload_coeff)
  , length(14.7)
  , num_axis(4)
  , J_axis(2.0)
  , wheel_diameter(0.95)
  , R1(0.0)
  , R2(0.0)
  , s(5)
  , b0(0.0)
  , b1(0.0)
  , b2(0.0)
  , b3(0.0)
  , q0(24.0)
  , inc(0.0)
  , curv(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle::~Vehicle()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::init(QString cfg_path)
{
    loadConfiguration(cfg_path);

    initialization();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setIndex(int idx)
{
    this->idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setInclination(double inc)
{
    this->inc = inc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setCurvature(double curv)
{
    this->curv = curv;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setForwardForce(double R1)
{
    this->R1 = R1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setBackwardForce(double R2)
{
    this->R2 = R2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setActiveCommonForce(int idx, double value)
{
    if (static_cast<size_t>(idx) < Q_a.size())
        Q_a[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setReactiveCommonForce(int idx, double value)
{
    if (static_cast<size_t>(idx) < Q_r.size())
        Q_r[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setPayloadCoeff(double payload_coeff)
{
    this->payload_coeff = Physics::cut(payload_coeff, 0.0, 1.0);
    full_mass = empty_mass + payload_mass * this->payload_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getMass() const
{
    return full_mass;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getLength() const
{
    return length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Vehicle::getDegressOfFreedom() const
{
    return s;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::step(double t, double dt)
{
    (void) t;
    (void) dt;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
state_vector_t Vehicle::getAcceleration(state_vector_t &Y, double t)
{
    (void) t;

    state_vector_t a;
    a.resize(s);

    double v = Y[idx + s];
    double V = abs(v) * Physics::kmh;

    double sin_beta = inc / 1000.0;
    double G = full_mass * Physics::g * sin_beta;

    double w = b0 + (b1 + b2 * V + b3 * V * V) / q0;
    double wk = 700.0 * curv;
    double W = full_mass * Physics::g * (w + wk) / 1000.0;

    double sumCreepForces = 0;

    double rk = wheel_diameter / 2.0;

    for (size_t i = 1; i <= static_cast<size_t>(num_axis); i++)
    {
        double creepForce = (Q_a[i] - Physics::fricForce(Q_r[i], Y[idx + s + i])) / rk;
        sumCreepForces += creepForce;
    }

    double Fr = Physics::fricForce(W + Q_r[0], v);

    a[0] = (Q_a[0] - Fr + R1 - R2 + sumCreepForces - G) / full_mass;

    for (size_t i = 0; i < static_cast<size_t>(num_axis); i++)
        a[i] = a[0] / rk;

    return a;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::initialization()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadConfig(QString cfg_path)
{
    (void) cfg_path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadConfiguration(QString cfg_path)
{
    loadConfig(cfg_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle *loadVehicle(QString lib_path)
{
    Vehicle *vehicle = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetVehicle getVehicle = (GetVehicle) lib.resolve("getVehicle");

        if (getVehicle)
        {
            vehicle = getVehicle();
        }
    }

    return vehicle;
}
