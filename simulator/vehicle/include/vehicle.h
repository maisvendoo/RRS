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

#ifndef     VEHICLE_H
#define     VEHICLE_H

#include    <QObject>
#include    <QtGlobal>

#include    "solver-types.h"

#if defined(VEHICLE_LIB)
    #define VEHICLE_EXPORT  Q_DECL_EXPORT
#else
    #define VEHICLE_EXPORT  Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VEHICLE_EXPORT Vehicle : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit Vehicle(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Vehicle();

    /// Vehicle initialization
    void init(QString cfg_path);

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

    void setRailwayCoord(double value);

    void setVelocity(double value);

    void setWheelAngle(size_t i, double value);
    void setWheelOmega(size_t i, double value);


    /// Get vehicle index
    int getIndex() const;

    /// Get vehicle mass
    double getMass() const;

    /// Get vehicle length
    double getLength() const;

    /// Get degrees of freedom
    int getDegressOfFreedom() const;

    /// Get wheel diameter
    double getWheelDiameter() const;

    double getRailwayCoord() const;

    double getVelocity() const;

    double getWheelAngle(size_t i);

    double getWheelOmega(size_t i);

    /// Common acceleration calculation
    virtual state_vector_t getAcceleration(state_vector_t &Y, state_vector_t &dYdt, double t);

    ///
    void integrationPreStep(state_vector_t &Y, state_vector_t &dYdt, double t);

    ///
    void integrationStep(state_vector_t &Y, state_vector_t &dYdt, double t, double dt);

    ///
    void integrationPostStep(state_vector_t &Y, state_vector_t &dYdt, double t);

signals:

    void logMessage(QString msg);

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

    /// Init railway coordinate
    double railway_coord0;
    /// Railway coordinate
    double railway_coord;
    /// Body velocity
    double velocity;
    /// Wheels rotation angles
    std::vector<double> wheel_rotation_angle;
    /// Wheels angular velocities
    std::vector<double> wheel_omega;

    double  b0;
    double  b1;
    double  b2;
    double  b3;
    double  q0;

    /// Vertical profile inclination
    double  inc;
    /// Railway curvature
    double  curv;

    /// Active common forces
    state_vector_t  Q_a;
    /// Reactive common forces
    state_vector_t  Q_r;

    /// User defined initialization
    virtual void initialization();

    /// User defined configuration load
    virtual void loadConfig(QString cfg_path);

    /// User defined step prepare
    virtual void preStep(double t);

    /// Internal ODE integration step
    virtual void step(double t, double dt);

    /// User define step result processing
    virtual void postStep(double t);

private:

    /// Default configuration load
    void loadConfiguration(QString cfg_path);

    /// Load main resistence coefficients
    void loadMainResist(QString cfg_path, QString main_resist_cfg);
};

/*!
 * \typedef
 * \brief Signature for getVehicle() function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Vehicle* (*GetVehicle)();

/*!
 * \def
 * \brief Macro for generate getVehicle() function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_VEHICLE(ClassName) \
    extern "C" Q_DECL_EXPORT Vehicle *getVehicle() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load vehicle from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Vehicle *loadVehicle(QString lib_path);

#endif // VEHICLE_H
