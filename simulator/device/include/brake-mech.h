//------------------------------------------------------------------------------
//
//      Vehicle brake mechanism abstract class
//      (c) maisvendoo, 15/02/2019
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Vehicle brake mechanism abstract class
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 15/02/2019
 */

#ifndef     BRAKE_MECH_H
#define     BRAKE_MECH_H

#include    "device.h"

/*!
 * \class
 * \brief Brake mechanics
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeMech : public Device
{
public:

    BrakeMech(QObject *parent = Q_NULLPTR);

    ~BrakeMech();

    /// Set air flow in/out brake cylinder
    void setAirFlow(double Q);

    /// Set effective brake radius
    void setEffFricRadius(double radius);

    /// Set wheel diameter
    void setWheelDiameter(double diameter);

    /// Set current vehicle velocity
    void setVelocity(double v);

    /// Get brake torque by one axis
    double getBrakeTorque() const;

    /// Get brake cylinder pressure
    double getBrakeCylinderPressure() const;

    double getShoeForce() const;

protected:

    /// Number of shoes for one axis
    int     shoesAxis;

    /// Number of brake cylinders
    int     cylNum;

    /// Air flow into/from brake cylinders
    double  Q;

    /// Axis brake torque
    double  brakeTorque;

    /// Dead volume of brake cylinder
    double  V0;

    /// Area of forcer
    double  S;

    /// Brake cylinder diameter
    double  cylDiam;

    ///
    double  Lmax;

    /// Brake press force
    double  K;

    /// Effective radius of friction
    double  effRadius;

    /// Wheel diameter
    double  wheelDiameter;

    /// Current vehicle velocity
    double  velocity;

    /// Shoes type (iron/iron-ph/composite)
    QString shoeType;

    /// Максимальное действительное нажатие на колодку
    double  Kmax;

    /// Максимальное давление в ТЦ
    double  p_max;

    /// Давление в ТЦ - начало движения поршня из отпускного положения
    double  p_begin;

    /// Давление в ТЦ - окончание движения поршня в тормозное положение
    double  p_end;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

protected:

    /// Load configuration
    virtual void load_config(CfgReader &cfg);

    /// Friction coefficient
    double phi(double K, double v);
};

/*!
 * \typedef
 * \brief Get brake mechics model function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeMech* (*GetBrakeMech)();

/*!
 * \def
 * \brief Macro for generation getBrakeMech() function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_MECH(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeMech *getBrakeMech() \
    { \
        return new (ClassName) (); \
    }

/*!
 * \fn
 * \brief Loading of brake mechanism module
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeMech *loadBrakeMech(QString lib_path);

#endif // BRAKEMECH_H
