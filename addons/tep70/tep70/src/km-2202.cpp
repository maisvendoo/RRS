#include    "km-2202.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::ControllerKM2202(QObject *parent) : Device(parent)
  , ms_delay(0.2)
  , rs_delay(0.2)
  , ms_position(MS_ZERO)
  , ms_dir(0)
  , rs_position(RS_ZERO)
  , rs_dir(0)
{
    main_shaft_timer.firstProcess(true);
    connect(&main_shaft_timer, &Timer::process,
            this, &ControllerKM2202::slotRotateMainShaft);

    revers_shaft_timer.firstProcess(true);
    connect(&revers_shaft_timer, &Timer::process,
            this, &ControllerKM2202::slotRotateReversShaft);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::~ControllerKM2202()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControllerKM2202::getMainShaftPos() const
{
    return static_cast<float>(ms_position) / MS_MAX_POSITION;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::ode_system(const state_vector_t &Y,
                                  state_vector_t &dYdt,
                                  double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    int timeout = 200;

    if (cfg.getInt(secName, "MainShaftDelay", timeout))
    {
        ms_delay = static_cast<double>(timeout) / 1000.0;
    }

    main_shaft_timer.setTimeout(ms_delay);

    timeout = 200;

    if (cfg.getInt(secName, "ReversShaftDelay", timeout))
    {
        rs_delay = static_cast<double>(timeout) / 1000.0;
    }

    revers_shaft_timer.setTimeout(rs_delay);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_A) || getKeyState(KEY_D) )
    {
        if (getKeyState(KEY_A))
        {
            ms_dir = 1;
            main_shaft_timer.start();
        }

        if (getKeyState(KEY_D))
        {
            if (!isControl())
            {
                ms_dir = -1;
                main_shaft_timer.start();
            }
            else
            {
                ms_position = MS_ZERO;
            }
        }
    }
    else
    {
        main_shaft_timer.stop();
    }


    if (getKeyState(KEY_W) || getKeyState(KEY_S))
    {
        if (getKeyState(KEY_W))
            rs_dir = 1;

        if (getKeyState(KEY_S))
            rs_dir = -1;

        revers_shaft_timer.start();
    }
    else
    {
        revers_shaft_timer.stop();
    }


    main_shaft_timer.step(t, dt);
    revers_shaft_timer.step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::slotRotateMainShaft()
{
    // Механическая блокировка поворота главного вала
    // в нулевом положении реверсивного
    if (rs_position == RS_ZERO)
    {
        return;
    }

    ms_position += ms_dir;
    ms_position = cut(ms_position, static_cast<int>(MS_ZERO), static_cast<int>(MS_MAX_POSITION));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::slotRotateReversShaft()
{
    // Механическая блокировка реверсивки на рабочих позициях
    if (ms_position != MS_ZERO)
    {
        return;
    }

    rs_position += rs_dir;
    rs_position = cut(rs_position, static_cast<int>(RS_BACKWARD), static_cast<int>(RS_FORWARD));
}
