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
#include    <QMap>
#include    <QMutex>

#include    "solver-types.h"
#include    "key-symbols.h"

#include    "vehicle-signals.h"
#include    "control-signals.h"
#include    "feedback-signals.h"

#include    "alsn-struct.h"

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
    void setIndex(size_t idx);

    /// Set inclination
    void setInclination(double inc);

    /// Set curvature
    void setCurvature(double curv);

    /// Set friction coefficient between wheel and rail
    void setFrictionCoeff(double coeff);

    /// Set direction
    void setDirection(int dir);

    /// Set orientation
    void setOrientation(int orient);

    /// Set forward coupling force
    void setForwardForce(double R);

    /// Set backward coupling force
    void setBackwardForce(double R);

    /// Set active common force
    void setActiveCommonForce(size_t idx, double value);

    /// Set reactive common force
    void setReactiveCommonForce(size_t idx, double value);

    /// Set payload level
    void setPayloadCoeff(double payload_coeff);

    void setRailwayCoord(double value);

    void setVelocity(double value);

    void setWheelAngle(size_t i, double value);
    void setWheelOmega(size_t i, double value);

    void setPrevVehicle(Vehicle *vehicle);
    void setNextVehicle(Vehicle *vehicle);

    void setConfigDir(QString config_dir);

    /// Get vehicle index
    size_t getIndex() const;

    /// Get orientation
    int getOrientation() const;

    /// Get vehicle mass
    double getMass() const;

    /// Get vehicle length
    double getLength() const;

    /// Get degrees of freedom
    size_t getDegressOfFreedom() const;

    /// Get wheel diameter
    double getWheelDiameter() const;

    double getRailwayCoord() const;

    double getVelocity() const;

    double getWheelAngle(size_t i);

    double getWheelOmega(size_t i);

    bool getDiscreteSignal(size_t i);

    float getAnalogSignal(size_t i);

    std::array<bool, MAX_DISCRETE_SIGNALS> getDiscreteSignals();
    std::array<float, MAX_ANALOG_SIGNALS> getAnalogSignals();

    /// Common acceleration calculation
    virtual state_vector_t getAcceleration(state_vector_t &Y, double t);

    ///
    void integrationPreStep(state_vector_t &Y, double t);

    virtual void keyProcess();

    virtual void hardwareProcess();

    ///
    void integrationStep(state_vector_t &Y, double t, double dt);

    ///
    void integrationPostStep(state_vector_t &Y, double t);

    ///
    double getBrakepipeBeginPressure() const;

    ///
    double getBrakepipeAuxRate() const;

    ///
    void setBrakepipePressure(double pTM);

    /// Задать поток воздуха в тормозную магистраль спереди
    void setBrakepipeFlowFwd(double flow);

    /// Задать поток воздуха в тормозную магистраль сзади
    void setBrakepipeFlowBwd(double flow);

    /// Давление воздуха в тормозной магистрали спереди
    double getBrakepipePressureFwd() const;

    /// Давление воздуха в тормозной магистрали сзади
    double getBrakepipePressureBwd() const;

    QString getDebugMsg() const;

    /// vehicle get sounds directory
    QString getSoundsDir() const;

    /// Init vehicle brake devices
    virtual void initBrakeDevices(double p0, double pTM, double pFL);

    void setUks(double value);

    void setCurrentKind(int value);

    void setEPTControl(size_t i, double value);

    double getEPTCurrent(size_t i);

    double getEPTControl(size_t i);

    void setASLN(alsn_info_t alsn_info);

    float getFwdOutput(size_t index) const;

    void setFwdInput(size_t index, float value);

    float getBwdOutput(size_t index) const;

    void setBwdInput(size_t index, float value);

    void setRouteDir(QString route_dir) { this->route_dir = route_dir; }

public slots:

    void receiveData(QByteArray data);

    void getControlSignals(control_signals_t control_signals);

signals:

    void logMessage(QString msg);

    void soundPlay(QString name);

    void soundStop(QString name);

    void soundSetVolume(QString name, int volume);

    void soundSetPitch(QString name, float pitch);

    void volumeCurveStep(QString name, float param);

    void sendFeedBackSignals(feedback_signals_t feedback_signals);

    void sendCoord(double x);

protected:

    /// Current route directory
    QString route_dir;

    /// Vehicle ODE system index
    size_t     idx;

    /// Empty vehicle mass (without payload)
    double  empty_mass;
    /// Full payload mass
    double  payload_mass;
    /// Payload coefficient (0.0 - empty, 1.0 - full payload)
    double  payload_coeff;
    /// Full vehicle mass
    double  full_mass;
    /// Vehicle sounds directory
    QString soundDirectory;

    /// Length between coupling's axis
    double  length;

    /// Numder of axis
    size_t     num_axis;
    /// Axis moment of inertia
    double  J_axis;
    /// Wheel diameter
    double  wheel_diameter;
    /// Wheel radius
    double  rk;

    /// Forward coupling force
    double  R1;
    /// Backward coupling force
    double  R2;
    /// Gravity force from profile inclination
    double  G_force;
    /// Max wheel friction force
    std::vector<double>  wheel_fric_max;
    /// Max friction force
    double fric_max;

    /// Number of degrees of freedom
    size_t  s;

    /// Init railway coordinate
    double railway_coord0;
    /// Railway coordinate
    double railway_coord;
    /// Body velocity
    double velocity;

    double  b0;
    double  b1;
    double  b2;
    double  b3;
    double  q0;
    double  W_coef;
    double  W_coef_v;
    double  W_coef_v2;
    double  W_coef_curv;

    /// Vertical profile inclination
    double  inc;
    /// Railway curvature
    double  curv;
    /// Friction coefficient between wheel and rail
    double  Psi;

    /// Railway motion direction
    int     dir;
    /// Vehicle orientation
    int     orient;

    /// Pressure in begin of brakepipe
    double p0;
    /// Aux brakepipe pressure rate
    double auxRate;
    /// Brakepipe pressure
    double pTM;

    /// Поток воздуха в тормозную магистраль спереди
    double QTMfwd;
    /// Поток воздуха в тормозную магистраль сзади
    double QTMbwd;
    /// Давление воздуха в тормозной магистрали спереди
    double pTMfwd;
    /// Давление воздуха в тормозной магистрали сзади
    double pTMbwd;

    QString DebugMsg;

    Vehicle *prev_vehicle;
    Vehicle *next_vehicle;

    QString config_dir;

    /// Напряжение в КС
    double      Uks;

    /// Род тока в КС
    int         current_kind;

    /// Wheels rotation angles
    std::vector<double> wheel_rotation_angle;
    /// Wheels angular velocities
    std::vector<double> wheel_omega;

    /// Active common forces
    state_vector_t  Q_a;
    /// Reactive common forces
    state_vector_t  Q_r;
    /// Vehicle common acceleration
    state_vector_t  a;

    /// Keyboard state
    QMap<int, bool> keys;
    QMutex          keys_mutex;

    /// Discrete signals for outpput
    std::array<bool, MAX_DISCRETE_SIGNALS>  discreteSignal;
    /// Analog signals for output
    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    control_signals_t   control_signals;

    feedback_signals_t  feedback_signals;

    /// Линии управления ЭПТ
    std::vector<double> ept_control;

    /// Ток в линии управления ЭПТ
    std::vector<double> ept_current;

    /// Информация АЛСН
    alsn_info_t     alsn_info;

    /// Входные сигналы СМЕ
    std::array<float, INPUTS_NUMBER>    forward_inputs;
    /// Выходные сигналы СМЕ
    std::array<float, OUTPUTS_NUMBER>   forward_outputs;

    /// Входные сигналы СМЕ
    std::array<float, INPUTS_NUMBER>    backward_inputs;
    /// Выходные сигналы СМЕ
    std::array<float, OUTPUTS_NUMBER>   backward_outputs;

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

    virtual void hardwareOutput();

    /* Modkeys extended functions */

    bool isShift() const;

    bool isControl() const;

    bool isAlt() const;

    bool getKeyState(int key) const;

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
