//------------------------------------------------------------------------------
//
//      Abstract class for train devices
//      (c) maisvendoo, 27/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract class for train devices
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 27/12/2018
 */

#ifndef     DEVICE_H
#define     DEVICE_H

#include    <QObject>

#include    "solver-types.h"
#include    "physics.h"
#include    "CfgReader.h"
#include    "debug.h"

#if defined(DEVICE_LIB)
    #define DEVICE_EXPORT   Q_DECL_EXPORT
#else
    #define DEVICE_EXPORT   Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Deivce base class
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Device : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    Device(QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~Device();

    /// Step of ODE system solving
    virtual void step(double t, double dt);            

    /// Set state variable
    void setY(size_t i, double value);

    /// Get state variable
    double getY(size_t i) const;

    /// Read device config file
    virtual void read_config(const QString &path);

signals:

    /// Print debug info into file
    void DebugPrint(double t, const state_vector_t &Y);

protected:

    /// State vector
    state_vector_t y;
    /// Derivative of state vector
    state_vector_t dydt;

    state_vector_t y1;

    state_vector_t k1;
    state_vector_t k2;
    state_vector_t k3;
    state_vector_t k4;

    /// Config directory
    std::string cfg_dir;

    std::string modules_dir;

    /// Device model ODE system
    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) = 0;

    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);

    virtual void preStep(const state_vector_t &Y, double t);

    virtual void postStep(const state_vector_t &Y, double t);
};

#endif // DEVICE_H
