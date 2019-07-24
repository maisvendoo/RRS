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

#include    "device-export.h"

#include    <QObject>
#include    <QMap>

#include    "solver-types.h"
#include    "physics.h"
#include    "CfgReader.h"
#include    "debug.h"
#include    "control-signals.h"
#include    "feedback-signals.h"
#include    "key-symbols.h"
#include    "timer.h"
#include    "trigger.h"


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

    virtual void read_custom_config(const QString &path);

    QString getDebugMsg() const;

    ///
    void setControl(QMap<int, bool> keys,
                    control_signals_t control_signals = control_signals_t());

    ///
    feedback_signals_t getFeedback() const;

signals:

    /// Print debug info into file
    void DebugPrint(double t, const state_vector_t &Y);

    void soundPlay(QString name);

    void soundStop(QString name);

    void soundSetVolume(QString name, int volume);

    void soundSetPitch(QString name, float pitch);

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

    QString     DebugMsg;

    QMap<int, bool>     keys;
    control_signals_t   control_signals;

    feedback_signals_t  feedback;

    /// Device model ODE system
    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) = 0;

    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void postStep(state_vector_t &Y, double t);    

    virtual void stepKeysControl(double t, double dt);

    virtual void stepExternalControl(double t, double dt);

    bool getKeyState(int key) const;

private:

    void memory_alloc(int order);

    void stepControl(double t, double dt);
};

#endif // DEVICE_H
