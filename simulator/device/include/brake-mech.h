//------------------------------------------------------------------------------
//
//      Vehicle brake mechanism abstract class
//      (c) maisvendoo, 15/02/2019
//
//------------------------------------------------------------------------------
#ifndef     BRAKE_MECH_H
#define     BRAKE_MECH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeMech : public Device
{
public:

    BrakeMech(QObject *parent = Q_NULLPTR);

    ~BrakeMech();

    void setAxisNumber(int numAxis);

    void setAirFlow(double Q);

    void setEffFricRadius(double radius);

    void setVelocity(double v);

    double getBrakeTorque() const;

    double getBrakeCylinderPressure() const;

protected:

    /// Number of vehicle axis
    int     numAxis;

    /// Number of shoes for one brake cylinder
    int     shoesCyl;

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

    /// Brake mechanism force coeff
    double  ip;

    ///
    double  Lmax;

    /// Springs equialent force
    double  F0;

    double  K;

    double  effRadius;

    double  velocity;

    QString shoeType;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(const state_vector_t &Y, double t);

private:

    virtual void load_config(CfgReader &cfg);

    /// Friction coefficient
    double phi(double K, double v);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeMech* (*GetBrakeMech)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_MECH(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeMech *getBrakeMech() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeMech *loadBrakeMech(QString lib_path);

#endif // BRAKEMECH_H
