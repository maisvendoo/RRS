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
#include    "registrator.h"
#include    "control-signals.h"
#include    "feedback-signals.h"
#include    "key-symbols.h"
#include    "hysteresis.h"
#include    "hysteresis-relay.h"
#include    "linear-interpolation.h"
#include    "timer.h"
#include    "trigger.h"


/*!
 * \class
 * \brief Device base class
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

    /// Set linked state
    virtual void link();

    /// Set unlinked state
    virtual void unlink();

    /// Get linked state
    bool isLinked() const;

    /// Set name
    void setName(QString value);

    /// Get name
    QString getName() const;

    /// Set signal
    void setInputSignal(size_t idx, double value);

    /// Get signal
    double getOutputSignal(size_t idx) const;

    /// Set state variable
    void setY(size_t i, double value);

    /// Get state variable
    double getY(size_t i) const;

    /// Read device config file
    virtual void read_config(const QString &filename, const QString &dir_path = "");

    /// Временно вернул, для обратной совместимости
    virtual void read_custom_config(const QString &path);

    QString getDebugMsg() const;

    ///
    void setControl(QMap<int, bool> keys,
                    control_signals_t control_signals = control_signals_t());

    ///
    feedback_signals_t getFeedback() const;

    void setCustomConfigDir(const QString &path);

    QString getCustomConfigDir() const;

signals:

    /// Print debug info into file
    void DebugPrint(double t, const state_vector_t &Y);

    void soundPlay(QString name);

    void soundStop(QString name);

    void soundSetVolume(QString name, int volume);

    void soundSetPitch(QString name, float pitch);

protected:

    /// Name of this device
    QString name;
    /// State of link with other device
    bool is_linked;

    /// Input signals
    state_vector_t input_signals;
    /// Output signals
    state_vector_t output_signals;

    /// State vector
    state_vector_t y;
    /// Derivative of state vector
    state_vector_t dydt;

    size_t sub_step_num;
    size_t solver_type;
    enum {
        RK4 = 4,
        EULER2 = 2,
        EULER = 1
    };

    state_vector_t y1;

    state_vector_t k1;
    state_vector_t k2;
    state_vector_t k3;
    //state_vector_t k4;

    /// Name of directory with vehicle's custom configs
    QString custom_cfg_dir;

    QString DebugMsg;

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

    virtual void stepDiscrete(double t, double dt);

    bool getKeyState(int key) const;

    bool isShift() const;

    bool isControl() const;

    bool isAlt() const;

private:

    void load_configuration(CfgReader &cfg);

    void step_rk4(double t, double dt);

    void step_euler2(double t, double dt);

    void step_euler(double t, double dt);

    void memory_alloc(int order);

    void stepControl(double t, double dt);
};

#endif // DEVICE_H
