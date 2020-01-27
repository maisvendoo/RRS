#include "kmb2.h"
#include "iostream"
#include    <sstream>

//------------------------------------------------------------------------------
// Конструктор класса
//------------------------------------------------------------------------------
KMB2::KMB2(QObject *parent)
    : reverse_dir(0),
      traction_pos(0),
      velocity_pos(0),
      traction_level(0),
      velocity_level(0),
      reverse_state(0),
      traction_rate(0),
      velocity_rate(0),
      T(0.5),
      S1(0),
      S2(0),
      S3(0),
      old_state_W(false),
      old_state_S(false)
{
    tractionVelocityLevelTimer = new Timer(0.1, false, Q_NULLPTR);
    connect(tractionVelocityLevelTimer, &Timer::process, this, &KMB2::levelTimerHandler);

    // Запуск таймера
    tractionVelocityLevelTimer->start();

    reverseStateTimer = new Timer(0.3, false, Q_NULLPTR);
    connect(reverseStateTimer, &Timer::process, this, &KMB2::reverseTimerHandler);
}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
KMB2::~KMB2()
{

}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getReverseDir()
{
    return reverse_dir;
}









//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getTractionPosition()
{
    return  traction_pos;
}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getVelocityPosition()
{
    return velocity_pos;
}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getTractionLevel()
{
    return  traction_level;
}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getVelocityLevel()
{
    return velocity_level;
}

//------------------------------------------------------------------------------
// Деструктор класса
//------------------------------------------------------------------------------
float KMB2::getReverseState()
{
    return  reverse_state;
}

double KMB2::getPovorot()
{
    return getY(0);
}

double KMB2::getS3()
{
    return S3;
}

void KMB2::preStep(state_vector_t &Y, double t)
{
    S1 = Y[0] - 0.99;
    S2 = hs_p(S1);
    S3 = S2 + pf(reverse_state);
}

void KMB2::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    dYdt[0] = (nf(reverse_state) - Y[0])/T;
}

void KMB2::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getInt(secName, "traction_rate", traction_rate);
    cfg.getInt(secName, "velocity_rate", velocity_rate);
}

//------------------------------------------------------------------------------
// Обработчик клавиш
//------------------------------------------------------------------------------
void KMB2::stepKeysControl(double t, double dt)
{
    // Реверс движения!
    if (getKeyState(KEY_W))
    {
        reverse_dir = 1;
        if (!old_state_W)
        {
            reverseStateTimer->start();
            old_state_W = true;
        }
    }
    else {
        old_state_W = false;
    }

    if (getKeyState(KEY_S))
    {
        reverse_dir = -1;

        if (!old_state_S)
        {
            reverseStateTimer->start();
            old_state_S = true;
        }
    }
    else {
        old_state_S = false;
    }


    // Положение тяги
    if (getKeyState(KEY_A))
    {
        traction_pos = 1;
    }
    if(getKeyState(KEY_D))
    {
        traction_pos = -1;
    }
    if (!getKeyState(KEY_A) && !getKeyState(KEY_D))
        traction_pos = 0;

    // Положение скорости
    if(getKeyState(KEY_Q))
    {
        velocity_pos = 1;
    }
    if(getKeyState(KEY_E))
    {
        velocity_pos = -1;
    }
    if (!getKeyState(KEY_Q) && !getKeyState(KEY_E))
        velocity_pos = 0;

    // Обнуление
    if (isControl())
    {
        if (getKeyState(KEY_D))
        {
            traction_level = 0;
            velocity_level = 0;
        }
    }

    tractionVelocityLevelTimer->step(t, dt);
    reverseStateTimer->step(t, dt);
}

void KMB2::levelTimerHandler()
{
    // Если реверс равен нулю!
    if(reverse_state == 0)
        return;

    traction_level += traction_pos * traction_rate;
    traction_level = cut(traction_level, -1, 1);

    velocity_level += velocity_pos * velocity_rate;
    velocity_level = cut(velocity_level, 0, 1);
}

void KMB2::reverseTimerHandler()
{
    // Если тяга не равна нулю
    if (traction_level != 0)
        return;

    reverse_state += reverse_dir;
    reverse_state = cut(reverse_state, -1, 1);

    reverseStateTimer->stop();
}

