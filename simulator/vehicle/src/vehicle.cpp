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
#include    <QDir>
#include    <QFileInfo>

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
void Vehicle::setRailwayCoord(double value)
{
    railway_coord0 = railway_coord = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setVelocity(double value)
{
    velocity = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setWheelAngle(size_t i, double value)
{
    if (i < wheel_rotation_angle.size())
        wheel_rotation_angle[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setWheelOmega(size_t i, double value)
{
    if (i < wheel_omega.size())
        wheel_omega[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Vehicle::getIndex() const
{
    return idx;
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
double Vehicle::getWheelDiameter() const
{
    return wheel_diameter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getRailwayCoord() const
{
    return railway_coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getVelocity() const
{
    return velocity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelAngle(size_t i)
{
    if (i < wheel_rotation_angle.size())
        return wheel_rotation_angle[i];
    else
        return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelOmega(size_t i)
{
    if (i < wheel_omega.size())
        return wheel_omega[i];
    else
        return 0;
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

    for (size_t i = 1; i <= static_cast<size_t>(num_axis); i++)
        a[i] = a[0] / rk;

    return a;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPreStep(state_vector_t &Y, double t)
{
    (void) Y;
    preStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationStep(state_vector_t &Y, double t, double dt)
{
    integrationPreStep(Y, t);
    step(t, dt);
    integrationPostStep(Y, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPostStep(state_vector_t &Y, double t)
{
    railway_coord = Y[idx];
    velocity = Y[idx + s];

    /*for (size_t i = 0; i < wheel_rotation_angle.size(); i++)
    {
        wheel_rotation_angle[i] = Y[idx + i + 1];
        wheel_omega[i] = Y[idx + s + i + 1];
    }*/

    postStep(t);
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
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getDouble(secName, "EmptyMass", empty_mass);
        cfg.getDouble(secName, "PayloadMass", payload_mass);
        cfg.getDouble(secName, "Length", length);
        cfg.getDouble(secName, "WheelDiameter", wheel_diameter);
        cfg.getInt(secName, "NumAxis", num_axis);

        s = num_axis + 1;

        Q_a.resize(s);
        Q_r.resize(s);

        for (size_t i = 0; i < Q_a.size(); i++)
            Q_a[i] = Q_r[i] = 0;

        cfg.getDouble(secName, "WheelInertia", J_axis);

        QString main_resist_cfg = "";
        cfg.getString(secName, "MainResist", main_resist_cfg);

        loadMainResist(cfg_path, main_resist_cfg);
    }
    else
    {
        emit logMessage("ERROR: file " + cfg_path + "is't found");
    }

    loadConfig(cfg_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadMainResist(QString cfg_path, QString main_resist_cfg)
{
    QFileInfo info(cfg_path);
    QDir dir(info.path());
    dir.cdUp();
    dir.cdUp();
    QString file_path = dir.path() + QDir::separator() +
            "main-resist" + QDir::separator() +
            main_resist_cfg + ".xml";

    CfgReader cfg;

    if (cfg.load(file_path))
    {
        QString secName = "MainResist";

        cfg.getDouble(secName, "b0", b0);
        cfg.getDouble(secName, "b1", b1);
        cfg.getDouble(secName, "b2", b2);
        cfg.getDouble(secName, "b3", b3);
        cfg.getDouble(secName, "q0", q0);
    }
    else
    {
        emit logMessage("ERROR: file " + file_path + "is't found");
    }
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::preStep(double t)
{
    (void) t;
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
void Vehicle::postStep(double t)
{
    (void) t;
}
