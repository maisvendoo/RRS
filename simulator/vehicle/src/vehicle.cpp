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
#include    "Journal.h"

#include    <QLibrary>
#include    <QDir>
#include    <QFileInfo>
#include    <QDataStream>

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
  , rk(0.475)
  , R1(0.0)
  , R2(0.0)
  , s(5)
  , railway_coord0(0.0)
  , railway_coord(0.0)
  , velocity(0.0)
  , b0(0.0)
  , b1(0.0)
  , b2(0.0)
  , b3(0.0)
  , q0(24.0)
  , inc(0.0)
  , curv(0.0)
  , dir(1)
  , p0(0.0)
  , auxRate(0.0)
  , pTM(0.0)
  , DebugMsg(" ")
  , prev_vehicle(nullptr)
  , next_vehicle(nullptr)
  , config_dir("")
  , Uks(0.0)
  , current_kind(0)
{
    std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    std::fill(discreteSignal.begin(), discreteSignal.end(), false);

    std::fill(forward_inputs.begin(), forward_inputs.end(), 0.0f);
    std::fill(forward_outputs.begin(), forward_outputs.end(), 0.0f);

    std::fill(backward_inputs.begin(), backward_inputs.end(), 0.0f);
    std::fill(backward_outputs.begin(), backward_outputs.end(), 0.0f);
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
    Journal::instance()->info("Started base class Vehicle initialization...");
    loadConfiguration(cfg_path);
    Journal::instance()->info("Base class initialization finished");

    Journal::instance()->info("Call of Vehicle::initialize() method...");
    initialization();
    Journal::instance()->info("Custom initialization finished");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setIndex(size_t idx)
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
void Vehicle::setDirection(int dir)
{
    this->dir = dir;
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
void Vehicle::setActiveCommonForce(size_t idx, double value)
{
    if (idx < Q_a.size())
        Q_a[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setReactiveCommonForce(size_t idx, double value)
{
    if (idx < Q_r.size())
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
void Vehicle::setPrevVehicle(Vehicle *vehicle)
{
    prev_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setNextVehicle(Vehicle *vehicle)
{
    next_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setConfigDir(QString config_dir)
{
    this->config_dir = config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getIndex() const
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
size_t Vehicle::getDegressOfFreedom() const
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
bool Vehicle::getDiscreteSignal(size_t i)
{
    if (i < discreteSignal.size())
        return discreteSignal[i];
    else
        return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getAnalogSignal(size_t i)
{
    if (i < analogSignal.size())
        return analogSignal[i];
    else
        return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
state_vector_t Vehicle::getAcceleration(state_vector_t &Y, double t)
{
    (void) t;

    // Get body velocity from state vector
    double v = Y[idx + s];
    // Convert velocity to kmh
    double V = abs(v) * Physics::kmh;

    // Calculate gravity force from profile inclination
    double sin_beta = inc / 1000.0;
    double G = full_mass * Physics::g * sin_beta;

    // Calculate main resistence force
    double w = b0 + (b1 + b2 * V + b3 * V * V) / q0;
    double wk = 700.0 * curv;
    double W = full_mass * Physics::g * (w + wk) / 1000.0;


    // Calculate equvivalent wheel forces
    double sumEqWheelForce = 0;

    for (size_t i = 1; i <= static_cast<size_t>(num_axis); i++)
    {
        double eqWheelForce = (Q_a[i] - Physics::fricForce(Q_r[i], dir * Y[idx + s + i])) / rk;
        sumEqWheelForce += eqWheelForce;
    }

    // Calculate equvivalent resistence force
    double Fr = Physics::fricForce(W + Q_r[0], dir * v);

    // Vehicle body's acceleration
    *a.begin() = dir * (*Q_a.begin() - Fr + R1 - R2 + sumEqWheelForce - G) / ( full_mass + num_axis * J_axis / rk / rk);

    // Wheels angle accelerations
    auto end = a.end();
    for (auto accel_it = a.begin() + 1; accel_it != end; ++accel_it)
        *accel_it = *a.begin() / rk;

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
void Vehicle::keyProcess()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::hardwareProcess()
{
    hardwareOutput();
    emit sendFeedBackSignals(feedback_signals);
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

    for (size_t i = 0; i < wheel_rotation_angle.size(); i++)
    {
        wheel_rotation_angle[i] = Y[idx + i + 1];
        wheel_omega[i] = Y[idx + s + i + 1] * dir;
    }

    postStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getBrakepipeBeginPressure() const
{
    return p0 * Physics::MPa + Physics::pA;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getBrakepipeAuxRate() const
{
    return auxRate * Physics::MPa;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setBrakepipePressure(double pTM)
{
    this->pTM = pTM;
}

QString Vehicle::getDebugMsg() const
{
    return DebugMsg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::receiveData(QByteArray data)
{
    if (data.size() == 0)
        return;

    keys_mutex.lock();

    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> keys;

    keys_mutex.unlock();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::getControlSignals(control_signals_t control_signals)
{
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setCurrentKind(int value)
{
    current_kind = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setEPTControl(size_t i, double value)
{
    if (i < ept_control.size())
        ept_control[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getEPTCurrent(size_t i)
{
    if (i < ept_current.size())
        return ept_current[i];

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getEPTControl(size_t i)
{
    if (i < ept_control.size())
        return ept_control[i];

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setASLN(alsn_info_t alsn_info)
{
    this->alsn_info = alsn_info;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getFwdInput(size_t index) const
{
    if (index < forward_inputs.size())
        return forward_inputs[index];

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setFwdOutput(size_t index, float value)
{
    if (index < forward_outputs.size())
        forward_outputs[index] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getBwdInput(size_t index) const
{
    if (index < backward_inputs.size())
        return backward_inputs[index];

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setBwdOutput(size_t index, float value)
{
    if (index < backward_outputs.size())
        backward_outputs[index] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setUks(double value)
{
    Uks = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::array<bool, MAX_DISCRETE_SIGNALS> Vehicle::getDiscreteSignals()
{
    return discreteSignal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::array<float, MAX_ANALOG_SIGNALS> Vehicle::getAnalogSignals()
{
    return analogSignal;
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
        Journal::instance()->info("Loaded config file: " + cfg_path);

        QString secName = "Vehicle";

        cfg.getDouble(secName, "EmptyMass", empty_mass);
        cfg.getDouble(secName, "PayloadMass", payload_mass);
        cfg.getDouble(secName, "Length", length);
        cfg.getDouble(secName, "WheelDiameter", wheel_diameter);
        cfg.getString(secName, "SoundDir", soundDirectory);


        Journal::instance()->info(QString("EmptyMass: %1 kg").arg(empty_mass));
        Journal::instance()->info(QString("PayloadMass: %1 kg").arg(payload_mass));
        Journal::instance()->info(QString("Length: %1 m").arg(length));
        Journal::instance()->info(QString("WheelDiameter: %1 m").arg(wheel_diameter));
        Journal::instance()->info(QString("SoundsDirectory: " + soundDirectory));

        rk = wheel_diameter / 2.0;

        int tmp = 0;
        cfg.getInt(secName, "NumAxis", tmp);        

        num_axis = static_cast<size_t>(tmp);
        wheel_rotation_angle.resize(num_axis);
        wheel_omega.resize(num_axis);

        Journal::instance()->info(QString("NumAxis: %1").arg(num_axis));

        s = num_axis + 1;

        Q_a.resize(s);
        Q_r.resize(s);
        a.resize(s);

        for (size_t i = 0; i < Q_a.size(); i++)
            Q_a[i] = Q_r[i] = 0;

        cfg.getDouble(secName, "WheelInertia", J_axis);

        Journal::instance()->info(QString("WheelInertia: %1 kg*m^2").arg(J_axis));

        QString main_resist_cfg = "";
        cfg.getString(secName, "MainResist", main_resist_cfg);

        loadMainResist(cfg_path, main_resist_cfg);
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " is't found");
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

        Journal::instance()->info("Main resist formula: " + QString("w = %1 + (%2 + %3 * V + %4 * V^2) / %5")
                                  .arg(b0).arg(b1).arg(b2).arg(b3).arg(q0));
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
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
        GetVehicle getVehicle = reinterpret_cast<GetVehicle>(lib.resolve("getVehicle"));

        if (getVehicle)
        {
            vehicle = getVehicle();
        }
        else
        {
            Journal::instance()->error(lib.errorString());
        }
    }
    else
    {
        Journal::instance()->error(lib.errorString());
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::hardwareOutput()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isShift() const
{
    return getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isControl() const
{
    return getKeyState(KEY_Control_L) || getKeyState(KEY_Control_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isAlt() const
{
    return getKeyState(KEY_Alt_L) || getKeyState(KEY_Alt_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::getKeyState(int key) const
{
    auto it = keys.find(key);

    if ( it != keys.end() )
        return it.value();

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::initBrakeDevices(double p0, double pTM, double pFL)
{
    Q_UNUSED(p0)
    Q_UNUSED(pTM)
    Q_UNUSED(pFL)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getSoundsDir() const
{
    return soundDirectory;
}
