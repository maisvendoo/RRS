#include    "motor-fan.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotorFan::MotorFan(size_t idx, QObject *parent) : Device(parent)
  , idx(idx)
  , Mmax(611.5)
  , s_kr_nom(0.08)
  , Un(380.0)
  , U_power(0.0)
  , omega0(157.08)
  , omega_nom(152.89)
  , kr(0.0154)
  , J(0.5)
  , is_no_ready(1.0)
  , f(50.0)
  , is_low_freq(true)  
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotorFan::~MotorFan()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotorFan::setU_power(double value)
{
    //QString lowSndName = QString("Motor_Fan%1_low").arg(idx);
    //QString normSndName = QString("Motor_Fan%1_norm").arg(idx);
    sound_state_t *cur_state = &sound_state[LOW_FREQ];

    if (f < 17)
    {
        if (!is_low_freq)
        {
            sound_state[HIGH_FREQ].play(false);
            cur_state = &sound_state[LOW_FREQ];

            if (!is_no_ready)
                cur_state->play(true);

            is_low_freq = true;
        }
    }
    else
    {
        if (is_low_freq)
        {
            sound_state[LOW_FREQ].play(false);
            cur_state = &sound_state[HIGH_FREQ];


            if (!is_no_ready)
                cur_state->play(true);

            is_low_freq = false;
        }
    }


    if (floor(value) > 0 && floor(U_power) == 0)
    {
        cur_state->play(true);
        is_no_ready = false;
    }

    if (floor(value) == 0 && floor(U_power) > 0)
    {
        cur_state->play(false);
        is_no_ready = true;
    }


    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MotorFan::isNoReady() const
{
    return is_no_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float MotorFan::getSoundSignal(size_t freq) const
{
    return sound_state[freq].createSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotorFan::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    //QString sndName = QString("Motor_Fan%1").arg(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotorFan::ode_system(const state_vector_t &Y,
                          state_vector_t &dYdt,
                          double t)
{
    Q_UNUSED(t)

    int p = 4; // число пар полюсов
    omega0 = 2 * Physics::PI * f / p;

    // Расчитывает текущее скольжение ротора
    double s = 1 - Y[0] / omega0;

    // Рачитываем максимальный момент при данном напряжении питания
    double M_maximal = Mmax * pow(U_power / Un, 2.0) * fn / f;

    double s_kr = s_kr_nom * fn / f;

    // Расчитываем электромагнитный момент (формула Клосса)
    double Ma = 2 * M_maximal / ( s / s_kr + s_kr / s );

    // Рассчитываем аэродинамический момент сопротивления
    double Mr = kr * Y[0] * Y[0] * Physics::sign(Y[0]);

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotorFan::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
