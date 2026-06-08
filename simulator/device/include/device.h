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

#include    <device-export.h>

#include    <QObject>
#include    <QMap>

#include    <solver-types.h>
#include    <sound-signal.h>
#include    <CfgReader.h>

#include    <control-signals.h>
#include    <feedback-signals.h>

#include    <devices-headers.h>

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
    Device(QObject *parent = nullptr);

    /// Destructor
    virtual ~Device() override;

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
    double getY(size_t i) const noexcept;

    /// Read device config file
    virtual void read_config(const QString &filename, const QString &dir_path = "");

    virtual QString getDebugMsg() const;

    ///
    virtual void setControl(std::set<uint16_t>* keys = nullptr,
                            control_signals_t* control_signals = nullptr);

    ///
    void setFeedbackPointer(feedback_signals_t* feedback_ptr);

    /// Device's sound state
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Device's sound state (as a single float value, see common-headers/sound-signal.h)
    virtual float getSoundSignal(size_t idx = 0) const;

    void setCustomConfigDir(const QString &path);

    QString getCustomConfigDir() const;

protected:

    /// Name of this device
    QString name = "";
    /// State of link with other device
    bool is_linked = false;

    int sub_step_num = 1;
    double max_step_dt = 0.0;

    /// Input signals
    state_vector_t input_signals;
    /// Output signals
    state_vector_t output_signals;

    /// State vector
    state_vector_t y;
    /// Derivative of state vector
    state_vector_t dydt;

    /// Name of directory with vehicle's custom configs
    QString custom_cfg_dir = "";

    std::set<std::uint16_t>*  pressed_keys = nullptr;
    control_signals_t*   control_signals = nullptr;

    feedback_signals_t*  feedback = nullptr;

    /// Device model ODE system
    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) = 0;

    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void postStep(state_vector_t &Y, double t);

    virtual void stepKeysControl(double t, double dt);

    virtual void stepExternalControl(double t, double dt);

    virtual void stepDiscrete(double t, double dt);

private:

    void load_configuration(CfgReader &cfg);

    void memory_alloc(int order);
};

#endif // DEVICE_H
