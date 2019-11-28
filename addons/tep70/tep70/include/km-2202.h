//------------------------------------------------------------------------------
//
//      Контроллер машиниста КМ 2202
//
//
//------------------------------------------------------------------------------
#ifndef     KM_2202_H
#define     KM_2202_H

#include    "device.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControllerKM2202 : public Device
{
public:

    ControllerKM2202(QObject *parent = Q_NULLPTR);

    ~ControllerKM2202();

    /// Вернуть текущую позицию штурвала
    int getPositionNumber() const { return ms_position; }

    /// Вернуть текущую позицию реверсивки
    int getReversState() const { return rs_position; }

    /// Вернуть положение главного вала
    float getMainShaftPos() const;

private:

    enum
    {
        MS_ZERO = 0,
        MS_MAX_POSITION = 15
    };

    enum
    {
        RS_FORWARD = 1,
        RS_ZERO = 0,
        RS_BACKWARD = -1
    };

    double  ms_delay;

    double  rs_delay;

    /// Положение главного вала
    int     ms_position;

    /// Направление вращения главного вала
    int     ms_dir;

    /// Положение реверсивного вала
    int     rs_position;

    /// Направление вращения реверсивного вала
    int     rs_dir;

    bool is_inc;

    Timer   main_shaft_timer;

    Timer   revers_shaft_timer;


    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

private slots:

    void slotRotateMainShaft();

    void slotRotateReversShaft();
};

#endif // KM2202_H
