//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     VEHICLE_H
#define     VEHICLE_H

#include    <QObject>

#include    "solver-types.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Vehicle : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit Vehicle(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Vehicle();

    /// Set vehicle index
    void setIndex(int idx);

    /// Set inclination
    void setInclination(double inc);

    /// Set curvature
    void setCurvature(double curv);

    /// Set forward coupling force
    void setForwardForce(double R1);

    /// Set backward coupling force
    void setBackwardForce(double R2);

    /// Set active common force
    void setActiveCommonForce(int idx, double value);

    /// Set reactive common force
    void setReactiveCommonForce(int idx, double value);

    /// Set payload level
    void setPayloadCoeff(double payload_coeff);

    /// Internal ODE integration step
    void step(double t, double dt);

protected:

    /// Vehicle ODE system index
    int     idx;

    /// Empty vehicle mass (without payload)
    double  empty_mass;
    /// Full payload mass
    double  payload_mass;
    /// Payload coefficient (0.0 - empty, 1.0 - full payload)
    double  payload_coeff;
    /// Full vehicle mass
    double  full_mass;

    /// Length between coupling's axis
    double  length;

    /// Numder of axis
    int     num_axis;
    /// Axis moment of inertia
    double  J_axis;
    /// Wheel diameter
    double  wheel_diameter;

    /// Forward coupling force
    double  R1;
    /// Backward coupling force
    double  R2;

    /// Number of degrees of freedom
    int     s;

    double  b0;
    double  b1;
    double  b2;
    double  b3;
    double  q0;

    /// Vertical profile inclination
    double  inc;
    /// Railsway curvature
    double  curv;

    state_vector_t  Q_a;

    state_vector_t  Q_r;

    /// User defined configuration load
    void loadConfig(QString cfg_path);

private:

    /// Default configuration load
    void loadConfiguration(QString cfg_path);
};

#endif // VEHICLE_H
