#include    "krm395.h"

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane395::BrakeCrane395(QObject *parent) : BrakeCrane (parent)
  , k_leek(0.0)
  , k_stab(0.0)
  , Vec(1.0)
  , A1(1.0)
  , k1(1.0)
  , k2(1.0)
  , k3(1.0)
  , k4(1.0)
  , T1(0.1)
  , T2(0.1)
  , K4_power(10.0)
  , Qbp(0.0)
  , old_input(0)
  , old_output(0)
  , pulse_II(true)
  , pulse_I(true)
  , t_old(0)
  , dt(0)
  , handle_pos(static_cast<int>(POS_II))
  , pos_angle(static_cast<int>(POS_II))
  , pos_delay(0.3)
  , min_pos(POS_I)
  , max_pos(POS_VI)
  , pos_duration(5.0)
  , dir(0)
  , pos_switch(true)
  , tau(0.0)

{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(pos.begin(), pos.end(), 0.0);
    pos[POS_II] = 1.0;

    positions_names << "I" << "II" << "III" << "IV" << "Va" << "V" << "VI";

    float max_angle = 115.0f;
    positions.push_back(0.0f);
    positions.push_back(35.0f / max_angle);
    positions.push_back(52.0f / max_angle);
    positions.push_back(59.0f / max_angle);
    positions.push_back(63.0f / max_angle);
    positions.push_back(80.0f / max_angle);
    positions.push_back(1.0f);

    incTimer = new Timer(pos_delay);
    decTimer = new Timer(pos_delay);

    //connect(incTimer, &Timer::process, this, &BrakeCrane395::inc);
    //connect(decTimer, &Timer::process, this, &BrakeCrane395::dec);

    connect(incTimer, SIGNAL(process()), this, SLOT(inc()), Qt::DirectConnection);
    connect(decTimer, SIGNAL(process()), this, SLOT(dec()), Qt::DirectConnection);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane395::~BrakeCrane395()
{
    delete debug_log;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::setPosition(int &position)
{
    position = cut(position, static_cast<int>(POS_I), static_cast<int>(POS_VI));

    std::fill(pos.begin(), pos.end(), 0.0);
    pos[static_cast<size_t>(position)] = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString BrakeCrane395::getPositionName()
{
    return positions_names[handle_pos];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float BrakeCrane395::getHandlePosition()
{
    return static_cast<float>(handle_pos) / 6.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::init(double pTM, double pFL)
{
    Q_UNUSED(pFL)

    y[0] = pTM;
    y[1] = pTM;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    t_old = t;

    pulse_II = static_cast<bool>(hs_n(Y[0] - p0));

    if ( (static_cast<int>(pos[POS_II]) == 1) && pulse_II )
    {
        Y[0] = cut(pf(2 * p0  - Y[1]), 0.0, pFL);
        pulse_II = false;
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::postStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)

    dt = t - t_old;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    double Q_charge = K[1] * (pFL - Y[1]) * pos[POS_I];

    double u1 = hs_n(Y[1] - p0);

    double Q_train = K[1] * (pFL - Y[1]) * u1 * pos[POS_II];

    double Q_stab = - k_stab * cut(k1 * Y[1], 0.0, 1.0) * pos[POS_II];

    double Q_brake = - K[5] * Y[1] * pos[POS_Va]
                     - K[6] * Y[1] * pos[POS_V]
                     - K[9] * Y[1] * pos[POS_VI];

    double Qer = Q_charge + Q_train + Q_stab + Q_brake;

    double s1 = A1 * (Y[0] - Y[1]);

    double K4 = 0;

    K4 = K[4] * (1.0 + k4 * pf(Y[0] - Y[1]));

    double u2 = cut(k2 * nf(s1), 0.0, 1.0) * (1.0 - pos[POS_VI]);
    double u3 = cut(k3 * pf(s1), 0.0, 1.0);

    double Qbp =   K[2] * (pFL - Y[0]) * u2
                 - K4 * Y[0] * u3
                 - K[7] * Y[0] * pos[POS_VI]
                 + K[8] * (pFL - Y[0]) * pos[POS_I];

    setBrakePipeFlow(Qbp);
    setEqResrvoirFlow(Qer);

    BrakeCrane::ode_system(Y, dYdt, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i <= MAX_FLOW_COEFFS; ++i)
    {
        QString param = QString("K%1").arg(i);

        cfg.getDouble(secName, param, K[i]);
    }

    cfg.getDouble(secName, "EqReservoirVolume", Ver);
    cfg.getDouble(secName, "LocoBrakePipeVolume", Vbp);
    cfg.getDouble(secName, "EqCameraVolume", Vec);
    cfg.getDouble(secName, "k_leek", k_leek);
    cfg.getDouble(secName, "k_stab", k_stab);
    cfg.getDouble(secName, "A1", A1);
    cfg.getDouble(secName, "k1", k1);
    cfg.getDouble(secName, "k2", k2);
    cfg.getDouble(secName, "k3", k3);
    cfg.getDouble(secName, "k4", k4);
    cfg.getDouble(secName, "T1", T1);
    cfg.getDouble(secName, "T2", T2);
    cfg.getDouble(secName, "K4_power", K4_power);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)

    if (getKeyState(KEY_Semicolon))
    {
        if (!decTimer->isStarted())
            decTimer->start();
    }
    else
    {
        decTimer->stop();
    }

    if (getKeyState(KEY_Quote))
    {
        if (!incTimer->isStarted())
            incTimer->start();
    }
    else
    {
        incTimer->stop();
    }

    if (isAlt())
    {
        if (getKeyState(KEY_1))
        {
            handle_pos = POS_I;
        }

        if (getKeyState(KEY_2))
        {
            handle_pos = POS_II;
        }

        if (getKeyState(KEY_3))
        {
            handle_pos = POS_III;
        }

        if (getKeyState(KEY_4))
        {
            handle_pos = POS_IV;
        }

        if (getKeyState(KEY_5))
        {
            handle_pos = POS_Va;
        }

        if (getKeyState(KEY_6))
        {
            handle_pos = POS_V;
        }

        if (getKeyState(KEY_7))
        {
            handle_pos = POS_VI;
        }
    }

    setPosition(handle_pos);

    incTimer->step(t, dt);
    decTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::inc()
{
   int old_pos = handle_pos;

   handle_pos++;

   handle_pos = cut(handle_pos, min_pos, max_pos);

   if (handle_pos != old_pos)
       emit soundPlay("Kran_395_ruk");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::dec()
{
    int old_pos = handle_pos;

    handle_pos--;

    handle_pos = cut(handle_pos, min_pos, max_pos);

    if (handle_pos != old_pos)
        emit soundPlay("Kran_395_ruk");
}

GET_BRAKE_CRANE(BrakeCrane395)
