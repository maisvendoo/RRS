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

#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle::Vehicle(QObject* parent) : QObject(parent)
{
    std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
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
void Vehicle::setModuleDir(QString module_dir)
{
    this->module_dir = module_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setModuleName(QString module_name)
{
    this->module_name = module_name;
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
void Vehicle::setConfigName(QString config_name)
{
    this->config_name = config_name;
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
void Vehicle::setModelIndex(size_t idx)
{
    this->model_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setTrainIndex(size_t idx)
{
    this->train_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setStateIndex(size_t idx)
{
    this->state_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setProfilePoint(profile_point_t point_data)
{
    this->profile_point_data = point_data;
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
void Vehicle::addForwardForce(double value)
{
    this->F_fwd += value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addBackwardForce(double value)
{
    this->F_bwd += value;
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
    this->payload_coeff = std::clamp(payload_coeff, 0.0, 1.0);
    full_mass = empty_mass + payload_mass * this->payload_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setTrainCoord(double value)
{
    train_coord = value;
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
void Vehicle::setPrevVehicle(Vehicle* vehicle)
{
    prev_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setNextVehicle(Vehicle* vehicle)
{
    next_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setNeedDebugMsg(bool is_needed)
{
    needDebugMsg = is_needed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getConfigDir() const
{
    return config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getConfigName() const
{
    return config_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getModuleDir() const
{
    return module_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getModuleName() const
{
    return module_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getModelIndex() const
{
    return model_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getTrainIndex() const
{
    return train_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getStateIndex() const
{
    return state_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_point_t* Vehicle::getProfilePoint()
{
    return &profile_point_data;
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
double Vehicle::getTrainCoord() const
{
    return train_coord;
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
Vehicle* Vehicle::getPrevVehicle()
{
    return prev_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle* Vehicle::getNextVehicle()
{
    return next_vehicle;
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
std::array<float, MAX_ANALOG_SIGNALS> Vehicle::getAnalogSignals()
{
    return analogSignal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t* Vehicle::getFwdConnectors()
{
    return &forward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t* Vehicle::getBwdConnectors()
{
    return &backward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_coord_list_t* Vehicle::getRailwayConnectors()
{
    return &railway_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
state_vector_t Vehicle::getAcceleration(state_vector_t& Y, double t, double dt)
{
    (void) t;

    // Direction & orientation
    int d = dir * orient;

    // Body velocity from state vector
    double v = d * Y[state_idx + s];
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
            double w = Y[state_idx + s + 1 + i];
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
void Vehicle::integrationPreStep(state_vector_t& Y, double t)
{
    train_coord = Y[state_idx];
    velocity = dir * orient * Y[state_idx + s];

    // Calculate gravity force from profile inclination
    double weight = full_mass * Physics::g;
    double sin_beta = profile_point_data.inclination / 1000.0;
    F_g = weight * sin_beta;

    F_fwd = 0.0;
    F_bwd = 0.0;

    if (num_axis > 0)
    {
        // Wheel-rail friction coefficient
        double psi_v = psi_coeff * wheelrailFriction(velocity);

        for (size_t i = 0; i < num_axis; i++)
        {
            wheel_rotation_angle[i] = Y[state_idx + 1 + i];
            wheel_omega[i] = Y[state_idx + s + 1 + i];
            psi[i] = psi_v;
            axis_load[i] = weight / static_cast<double>(num_axis);
        }
    }

    {
        std::lock_guard lock(keyboard_mutex);
        pressed_keys.clear();
        for (const auto& pressed_keys_at_cab : pressed_keys_by_cabine)
        {
            pressed_keys.insert(pressed_keys_at_cab.begin(), pressed_keys_at_cab.end());
        }
    }

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationStep(state_vector_t& Y, double t, double dt)
{
    (void) Y;
    step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPostStep(state_vector_t& Y, double t)
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
void Vehicle::setKeyboardControl(const uint8_t& cab_num, const std::vector<uint16_t>& pressed_keys)
{
    if (cab_num >= pressed_keys_by_cabine.size())
        return;

    std::lock_guard lock(keyboard_mutex);
    pressed_keys_by_cabine[cab_num].insert(pressed_keys.begin(), pressed_keys.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::resetKeyboardControl(const uint8_t& cab_num)
{
    if (cab_num >= pressed_keys_by_cabine.size())
        return;

    std::lock_guard lock(keyboard_mutex);
    pressed_keys_by_cabine[cab_num].clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setControlSignals(const control_signals_t& control_signals)
{
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
feedback_signals_t& Vehicle::getFeedBackSignals()
{
    return feedback_signals;
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
void Vehicle::setUks(double value)
{
    Uks = value;
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

        Journal::instance()->info(QString("EmptyMass: %1 kg").arg(empty_mass));
        Journal::instance()->info(QString("PayloadMass: %1 kg").arg(payload_mass));
        Journal::instance()->info(QString("Length: %1 m").arg(length));

        full_mass = empty_mass + payload_mass * payload_coeff;

        if (axis > 0)
        {
            num_axis = static_cast<size_t>(axis);

            wheel_rotation_angle.resize(num_axis);
            wheel_rotation_angle.shrink_to_fit();
            std::fill(wheel_rotation_angle.begin(), wheel_rotation_angle.end(), 0.0);

            wheel_omega.resize(num_axis);
            wheel_omega.shrink_to_fit();
            std::fill(wheel_omega.begin(), wheel_omega.end(), 0.0);

            wheel_diameter.resize(num_axis);
            wheel_diameter.shrink_to_fit();
            std::fill(wheel_diameter.begin(), wheel_diameter.end(), diameter);

            double tmp = diameter / 2.0;
            rk.resize(num_axis);
            rk.shrink_to_fit();
            std::fill(rk.begin(), rk.end(), tmp);

            J_axis.resize(num_axis);
            J_axis.shrink_to_fit();
            std::fill(J_axis.begin(), J_axis.end(), J);

            tmp = full_mass * Physics::g / static_cast<double>(num_axis);
            axis_load.resize(num_axis);
            axis_load.shrink_to_fit();
            std::fill(axis_load.begin(), axis_load.end(), tmp);

            psi.resize(num_axis);
            psi.shrink_to_fit();
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
    Q_a.shrink_to_fit();
    std::fill(Q_a.begin(), Q_a.end(), 0.0);

    Q_r.resize(s);
    Q_r.shrink_to_fit();
    std::fill(Q_r.begin(), Q_r.end(), 0.0);

    acceleration.resize(s);
    acceleration.shrink_to_fit();
    std::fill(acceleration.begin(), acceleration.end(), 0.0);
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
void Vehicle::addFwdConnector(Device* device)
{
    forward_connectors.push_back(device);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addBwdConnector(Device* device)
{
    backward_connectors.push_back(device);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addRailwayConnector(Device* device, double distance_from_center)
{
    device_coord_t dc;
    dc.device = device;
    dc.coord = std::clamp(distance_from_center, -length / 2.0, length / 2.0);
    railway_connectors.push_back(dc);
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

        cfg.getDouble(secName, "a", psi_a);
        cfg.getDouble(secName, "b", psi_b);
        cfg.getDouble(secName, "c", psi_c);
        cfg.getDouble(secName, "d", psi_d);
        cfg.getDouble(secName, "e", psi_e);

        Journal::instance()->info("Wheel model config: " + QString("%1")
                                  .arg(file_path));
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
    }
    Journal::instance()->info("Wheel friction coefficient formula: " + QString("psi = %1 + (%2 / (%3 + %4 * V)) + %5 * V")
                              .arg(psi_a).arg(psi_b).arg(psi_c).arg(psi_d).arg(psi_e));
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
                        + W_coef_curv * abs(profile_point_data.curvature)  );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::wheelrailFriction(double velocity)
{
    double abs_V = abs(velocity) * Physics::kmh;
    return psi_a + (psi_b / (psi_c + psi_d * abs_V)) + psi_e * abs_V;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isShift(int cab_num) const
{
    return getKeyState(KEY_Shift_L, cab_num) || getKeyState(KEY_Shift_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isControl(int cab_num) const
{
    return getKeyState(KEY_Control_L, cab_num) || getKeyState(KEY_Control_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isAlt(int cab_num) const
{
    return getKeyState(KEY_Alt_L, cab_num) || getKeyState(KEY_Alt_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::getKeyState(uint16_t key, int cab_num) const
{
    if (cab_num < 0)
    {
        return pressed_keys.count(key);
    }

    if (cab_num >= pressed_keys_by_cabine.size())
    {
        return false;
    }

    return pressed_keys_by_cabine[cab_num].count(key);
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
Vehicle* loadVehicle(QString lib_path)
{
    Vehicle* vehicle = nullptr;

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
