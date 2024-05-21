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
  , route_dir("")
  , soundDirectory("")
  , idx(0)
  , empty_mass(24000.0)
  , payload_mass(68000.0)
  , payload_coeff(0.0)
  , full_mass(empty_mass + payload_mass * payload_coeff)
  , length(14.7)
  , num_axis(0)
  , psi_coeff(1.0)
  , F_fwd(0.0)
  , F_bwd(0.0)
  , F_g(0.0)
  , s(1)
  , railway_coord0(0.0)
  , railway_coord(0.0)
  , velocity(0.0)
  , b0(0.0)
  , b1(0.0)
  , b2(0.0)
  , b3(0.0)
  , q0(24.0)
  , W_coef((b0 + b1 / q0) * Physics::g / 1000.0)
  , W_coef_v((b2 / q0) * Physics::g * Physics::kmh / 1000.0)
  , W_coef_v2((b3 / q0) * Physics::g * Physics::kmh * Physics::kmh / 1000.0)
  , W_coef_curv(700.0 * Physics::g / 1000.0)
  , a(0.0)
  , b(30.0)
  , c(100.0)
  , d(1.0)
  , e(0.0)
  , inc(0.0)
  , curv(0.0)
  , dir(1)
  , orient(1)
  , DebugMsg(" ")
  , prev_vehicle(nullptr)
  , next_vehicle(nullptr)
  , config_dir("")
  , Uks(0.0)
  , current_kind(0)
  , coupling_fwd(nullptr)
  , coupling_bwd(nullptr)
{
    std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    std::fill(discreteSignal.begin(), discreteSignal.end(), false);
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

    Journal::instance()->info("Call of Vehicle::initialization() method...");
    initialization();
    Journal::instance()->info("Custom initialization finished");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setRouteDir(QString route_dir)
{
    this->route_dir = route_dir;
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
void Vehicle::setInclination(double value)
{
    this->inc = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setCurvature(double value)
{
    this->curv = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setFrictionCoeff(double value)
{
    this->psi_coeff = value;
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
void Vehicle::setOrientation(int orient)
{
    this->orient = orient;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setForwardForce(double value)
{
    this->F_fwd = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setBackwardForce(double value)
{
    this->F_bwd = value;
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
int Vehicle::getOrientation() const
{
    return orient;
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
size_t Vehicle::getNumAxis() const
{
    return num_axis;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelDiameter(size_t i) const
{
    if (i < wheel_diameter.size())
        return wheel_diameter[i];
    else
        return 0;
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
device_list_t *Vehicle::getFwdConnectors()
{
    return &forward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t *Vehicle::getBwdConnectors()
{
    return &backward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
state_vector_t Vehicle::getAcceleration(state_vector_t &Y, double t, double dt)
{
    (void) t;

    // Direction & orientation
    int d = dir * orient;

    // Body velocity from state vector
    double v = d * Y[idx + s];
    double abs_v = abs(v);

    // Forces from wheels to vehicle body
    double F_wheels = 0.0;
    double R_wheels_fwd = 0.0;
    double R_wheels_bwd = 0.0;
    if (num_axis > 0)
    {
        auto a_it = acceleration.begin();
        auto Qa_it = Q_a.begin();
        auto Qr_it = Q_r.begin();
        for (size_t i = 0; i < num_axis; ++i)
        {
            ++a_it;
            ++Qa_it;
            ++Qr_it;
            // Wheel's angular velocity
            double w = Y[idx + s + 1 + i];
            double abs_w = abs(w);
            // Wheel's slip velocity
            double slip = w * rk[i] - v;
            double abs_slip = abs(slip);
            // Reduce friction coefficient between wheel and rail when it slips
            double psi_slip = psi[i] / (1.0 + abs_slip);

            // Active torque
            double wheel_a = *Qa_it;

            // Reactive torque
            double potential_r = pf(*Qr_it);
            double wheel_r = 0.0;
            if (abs_w > Physics::ZERO)
            {
                // Calculate maximum possible velocity change
                double dw_max = (potential_r / J_axis[i]) * dt;
                if (abs_w >= dw_max)
                {
                    wheel_r = potential_r * sign(w);
                    potential_r = 0.0;
                }
                else
                {
                    // If velocity will fall to zero, reduce reactive force
                    double e = abs_w / dw_max;
                    wheel_r = e * potential_r * sign(w);
                    potential_r = (1 - e) * potential_r;
                }
            }

            // Friction torque
            double potential_f = psi_slip * axis_load[i] * rk[i];
            double wheel_f = 0.0;
            if (abs_slip > Physics::ZERO)
            {
                // Calculate maximum possible velocity change
                double dslip_max = (potential_f / J_axis[i]) * dt;
                if (abs_slip >= dslip_max)
                {
                    wheel_f = potential_f * sign(slip);
                    potential_f = 0.0;
                }
                else
                {
                    // If slip will fall to zero, reduce friction force
                    double e = abs_slip / dslip_max;
                    wheel_f = e * potential_f * sign(slip);
                    potential_f = (1 - e) * potential_f;
                }
            }

            // Apply potential reactive forces
            double tmp_r = min(abs(wheel_a - wheel_f), potential_r) * sign(wheel_a - wheel_f);
            wheel_r += tmp_r;

            double tmp_f = min(abs(wheel_a - wheel_r), potential_f) * sign(wheel_a - wheel_r);
            wheel_f += tmp_f;

            // Apply wheel-rail forces to vehicle
            F_wheels += wheel_f / rk[i];
            R_wheels_fwd += min(potential_r - tmp_r, potential_f - tmp_f) / rk[i];
            R_wheels_bwd += min(potential_r + tmp_r, potential_f + tmp_f) / rk[i];

            // Calculate and apply wheel angle acceleration
            *a_it = (wheel_a - wheel_r - wheel_f) / J_axis[i];
        }
    }

    // Calculate main resistance force
    double W = mainResist(v);

    // Common body active force
    double force_a = *Q_a.begin() + F_wheels + F_fwd - F_bwd - F_g;

    // Common body reactive force
    double potential_fwd = W + pf(Q_r[0]) + R_wheels_fwd;
    double potential_bwd = W + pf(Q_r[0]) + R_wheels_bwd;
    double force_r = 0.0;
    if (abs_v < Physics::ZERO)
    {
        if (force_a > 0)
        {
            force_r = min(force_a, potential_fwd);
        }
        if (force_a < 0)
        {
            force_r = max(force_a, - potential_bwd);
        }
    }
    else
    {
        if (v > 0)
        {
            force_r = potential_fwd;
        }
        else
        {
            force_r = - potential_bwd;
        }

        // Prediction of velocity
        double dv = ((force_a - force_r) / full_mass) * dt;
        // If velocity will fall to zero, reduce reactive force after it
        if (sign(v) != sign(v + dv))
        {
            double e = abs(v / dv);
            force_r = e * force_r;
            if (force_a > 0.0)
            {
                force_r += (1 - e) * min(force_a, potential_fwd);
            }
            if (force_a < 0.0)
            {
                force_r += (1 - e) * max(force_a, - potential_bwd);
            }
        }
    }

    // Vehicle body's acceleration
    *acceleration.begin() = d * (force_a - force_r) / full_mass;

    return acceleration;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPreStep(state_vector_t &Y, double t)
{
    railway_coord = Y[idx];
    velocity = dir * orient * Y[idx + s];

    coupling_fwd->setCoord(Y[idx] + dir * orient * length / 2.0);
    coupling_fwd->setVelocity(Y[idx + s]);
    coupling_bwd->setCoord(Y[idx] - dir * orient * length / 2.0);
    coupling_bwd->setVelocity(Y[idx + s]);

    // Calculate gravity force from profile inclination
    double weight = full_mass * Physics::g;
    double sin_beta = inc / 1000.0;
    F_g = weight * sin_beta * orient;

    if (num_axis > 0)
    {
        // Wheel-rail friction coefficient
        double psi_v = psi_coeff * wheelrailFriction(velocity);

        for (size_t i = 0; i < num_axis; i++)
        {
            wheel_rotation_angle[i] = Y[idx + 1 + i];
            wheel_omega[i] = Y[idx + s + 1 + i];
            psi[i] = psi_v;
            axis_load[i] = weight / static_cast<double>(num_axis);
        }
    }

    emit sendCoord(railway_coord + dir * length / 2.0);

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

    coupling_fwd->step(t, dt);
    F_fwd = coupling_fwd->getForce();
    coupling_bwd->step(t, dt);
    F_bwd = coupling_bwd->getForce();

    step(t, dt);
    integrationPostStep(Y, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPostStep(state_vector_t &Y, double t)
{
    (void) Y;

    postStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
void Vehicle::setASLN(alsn_info_t alsn_info)
{
    this->alsn_info = alsn_info;
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
void Vehicle::loadConfiguration(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded config file: " + cfg_path);

        QString secName = "Vehicle";

        double diameter = 0.0;
        double J = 0.0;
        int axis = 0;
        cfg.getDouble(secName, "EmptyMass", empty_mass);
        cfg.getDouble(secName, "PayloadMass", payload_mass);
        cfg.getDouble(secName, "Length", length);
        cfg.getInt(secName, "NumAxis", axis);
        cfg.getDouble(secName, "WheelDiameter", diameter);
        cfg.getDouble(secName, "WheelInertia", J);
        cfg.getString(secName, "SoundDir", soundDirectory);

        Journal::instance()->info(QString("EmptyMass: %1 kg").arg(empty_mass));
        Journal::instance()->info(QString("PayloadMass: %1 kg").arg(payload_mass));
        Journal::instance()->info(QString("Length: %1 m").arg(length));

        full_mass = empty_mass + payload_mass * payload_coeff;

        if (axis > 0)
        {
            num_axis = static_cast<size_t>(axis);

            wheel_rotation_angle.resize(num_axis);
            std::fill(wheel_rotation_angle.begin(), wheel_rotation_angle.end(), 0.0);

            wheel_omega.resize(num_axis);
            std::fill(wheel_omega.begin(), wheel_omega.end(), 0.0);

            wheel_diameter.resize(num_axis);
            std::fill(wheel_diameter.begin(), wheel_diameter.end(), diameter);

            double tmp = diameter / 2.0;
            rk.resize(num_axis);
            std::fill(rk.begin(), rk.end(), tmp);

            J_axis.resize(num_axis);
            std::fill(J_axis.begin(), J_axis.end(), J);

            tmp = full_mass * Physics::g / static_cast<double>(num_axis);
            axis_load.resize(num_axis);
            std::fill(axis_load.begin(), axis_load.end(), tmp);

            psi.resize(num_axis);
            std::fill(psi.begin(), psi.end(), 0.30);

            Journal::instance()->info(QString("NumAxis: %1").arg(num_axis));
            Journal::instance()->info(QString("WheelDiameter: %1 m").arg(diameter));
            Journal::instance()->info(QString("WheelInertia: %1 kg*m^2").arg(J));
        }
        else
        {
            num_axis = 0;
            Journal::instance()->warning(QString("NumAxis is zero. Wheel's model will't used"));
        }

        Journal::instance()->info(QString("SoundsDirectory: " + soundDirectory));

        QString main_resist_cfg = "";
        cfg.getString(secName, "MainResist", main_resist_cfg);
        loadMainResist(cfg_path, main_resist_cfg);

        QString wheel_cfg = "default";
        cfg.getString(secName, "WheelRailFriction", wheel_cfg);
        loadWheelRailFriction(cfg_path, wheel_cfg);

        s = 1 + num_axis;

        loadConfig(cfg_path);
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " is't found");
    }

    Q_a.resize(s);
    Q_r.resize(s);
    acceleration.resize(s);
    std::fill(Q_a.begin(), Q_a.end(), 0.0);
    std::fill(Q_r.begin(), Q_r.end(), 0.0);
    std::fill(acceleration.begin(), acceleration.end(), 0.0);

    // Couplings
    coupling_fwd = new Coupling();
    coupling_fwd->couple();
    forward_connectors.push_back(coupling_fwd);
    coupling_bwd = new Coupling();
    coupling_bwd->couple();
    backward_connectors.push_back(coupling_bwd);
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

        mainResistCoeffs();
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
    }
    Journal::instance()->info("Main resist formula: " + QString("w = %1 + (%2 + %3 * V + %4 * V^2) / %5")
                              .arg(b0).arg(b1).arg(b2).arg(b3).arg(q0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadWheelRailFriction(QString cfg_path, QString wheel_cfg)
{
    QFileInfo info(cfg_path);
    QDir dir(info.path());
    dir.cdUp();
    dir.cdUp();
    QString file_path = dir.path() + QDir::separator() +
            "wheel-rail-friction" + QDir::separator() +
            wheel_cfg + ".xml";

    CfgReader cfg;

    if (cfg.load(file_path))
    {
        QString secName = "WheelModel";

        cfg.getDouble(secName, "a", a);
        cfg.getDouble(secName, "b", b);
        cfg.getDouble(secName, "c", c);
        cfg.getDouble(secName, "d", d);
        cfg.getDouble(secName, "e", e);

        Journal::instance()->info("Wheel model config: " + QString("%1")
                                  .arg(file_path));
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
    }
    Journal::instance()->info("Wheel friction coefficient formula: " + QString("psi = %1 + (%2 / (%3 + %4 * V)) + %5 * V")
                              .arg(a).arg(b).arg(c).arg(d).arg(e));
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
void Vehicle::mainResistCoeffs()
{
    double q = 1.0;
    if ((q0 > 1.0) && (num_axis > 0))
        q = full_mass / (1000.0 * static_cast<double>(num_axis));

    W_coef = (b0 + b1 / q) * Physics::g / 1000.0;
    W_coef_v = (b2 / q) * Physics::g * Physics::kmh / 1000.0;
    W_coef_v2 = (b3 / q) * Physics::g * Physics::kmh * Physics::kmh / 1000.0;
    W_coef_curv = 700.0 * Physics::g / 1000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::mainResist(double velocity)
{
    return full_mass * (  W_coef
                        + W_coef_v * abs(velocity)
                        + W_coef_v2 * velocity * velocity
                        + W_coef_curv * curv  );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::wheelrailFriction(double velocity)
{
    double abs_V = abs(velocity) * Physics::kmh;
    return a + (b / (c + d * abs_V)) + e * abs_V;
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
