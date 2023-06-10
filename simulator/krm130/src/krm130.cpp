#include    "krm130.h"

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane130::BrakeCrane130(QObject *parent) : BrakeCrane (parent)
  , k_leak(1.0e-6)
  , k_charge(1.0e-3)
  , k_stab(4.0e-6)
  , k_Va(1.0e-4)
  , k_V(1.0e-3)
  , k_VI(1.5e-3)
  , A(0.1)
  , K_charge(5.0e-2)
  , K_feed(2.0e-2)
  , K_atm(1.5e-2)
  , K_VI(8.0e-2)
  , handle_pos(static_cast<int>(POS_II))
  , pos_delay(0.3)
  , min_pos(POS_I)
  , max_pos(POS_VI)
{
    std::fill(pos.begin(), pos.end(), 0.0);
    pos[POS_II] = 1.0;

    positions_names << "I" << "II" << "III" << "IV" << "Va" << "V" << "VI";

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
BrakeCrane130::~BrakeCrane130()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::init(double pBP, double pFL)
{
    Q_UNUSED(pFL)

    setY(ER_PRESSURE, pBP);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::setHandlePosition(int &position)
{
    position = cut(position, static_cast<int>(POS_I), static_cast<int>(POS_VI));

    std::fill(pos.begin(), pos.end(), 0.0);
    pos[static_cast<size_t>(position)] = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString BrakeCrane130::getPositionName() const
{
    return positions_names[handle_pos];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeCrane130::getHandlePosition() const
{
    return static_cast<double>(handle_pos) / 6.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)
/*
    t_old = t;

    if ( (static_cast<int>(pos[POS_II]) == 1) && pulse_II )
    {
        Y[0] = cut(pf(2 * p0  - Y[1]), 0.0, pFL);
        pulse_II = false;
    }
    else
    {
        pulse_II = true;
    }
*/
    is_hold = static_cast<bool>(pos[POS_III] + pos[POS_IV]);
    is_brake = static_cast<bool>(pos[POS_Va] + pos[POS_V] + pos[POS_VI]);
}
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::postStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)

    dt = t - t_old;
}
*/
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    // Зарядка УР из ГР в I положении
    double Q_leak_er = - k_leak * Y[ER_PRESSURE];

    // Зарядка УР из ГР в I положении
    double Q_charge_er = pos[POS_I] * k_charge * (pFL - Y[ER_PRESSURE]);

    // Зарядка УР из ГР до зарядного давления в II положении
    double Q_train_er = pos[POS_II] * cut(p0 - Y[ER_PRESSURE], 0.0, k_charge) * (pFL - Y[ER_PRESSURE]);

    // Разрядка УР через стабилизатор
    double Q_stab_er = - pos[POS_II] * cut(Y[ER_PRESSURE], 0.0, k_stab);

    // Разрядка УР в тормозных положениях
    double Q_brake_er = - pos[POS_Va] * k_Va * Y[ER_PRESSURE]
                        - pos[POS_V] * k_V * Y[ER_PRESSURE]
                        - pos[POS_VI] * k_VI * Y[ER_PRESSURE];

    // Условное положение уравнительного поршня
    double s1 = A * (Y[ER_PRESSURE] - pBP);

    // Зарядка ТМ из ГР в I положении
    double Q_charge_bp = pos[POS_I] * K_charge * (pFL - pBP);

    // Управление ТМ от уравнительного поршня в II, IV, Va и V положениях
    double is_245a5 = pos[POS_II] + pos[POS_IV] + pos[POS_Va] + pos[POS_V];

    // Подзарядка ТМ из ГР от уравнительного поршня
    double Q_train_bp = is_245a5 * cut(s1, 0.0, K_feed) * (pFL - pBP);

    // Разрядка ТМ от уравнительного поршня
    double Q_brake_bp = is_245a5 * cut(s1, - K_atm, 0.0) * pBP;

    // Экстренная разрядка ТМ в VI положении
    double Q_emerg_bp = - K_VI * pBP * pos[POS_VI];

    // Суммарный поток в питательную магистраль
    QFL = - Q_charge_er - Q_train_er - Q_charge_bp - Q_train_bp;

    // Суммарный поток в тормозную магистраль
    QBP = Q_charge_bp + Q_train_bp + Q_brake_bp + Q_emerg_bp;

    // Суммарный поток в уравнительный резервуар
    setERflow(Q_leak_er + Q_charge_er + Q_train_er + Q_stab_er + Q_brake_er);

    BrakeCrane::ode_system(Y, dYdt, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "EqReservoirVolume", tmp);
    if (tmp > 0.0)
        Ver = tmp;

    cfg.getDouble(secName, "k_leak", k_leak);
    cfg.getDouble(secName, "k_charge", k_charge);
    cfg.getDouble(secName, "k_stab", k_stab);
    cfg.getDouble(secName, "k_Va", k_Va);
    cfg.getDouble(secName, "k_V", k_V);
    cfg.getDouble(secName, "k_VI", k_VI);

    cfg.getDouble(secName, "A", A);

    cfg.getDouble(secName, "K_charge", K_charge);
    cfg.getDouble(secName, "KF", K_feed);
    cfg.getDouble(secName, "K_atm", K_atm);
    cfg.getDouble(secName, "K_VI", K_VI);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::stepKeysControl(double t, double dt)
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

        // Воазврат ручки из сверхзарядки в поездное
        if (handle_pos == POS_I)
            handle_pos = POS_II;
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

    setHandlePosition(handle_pos);

    incTimer->step(t, dt);
    decTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane130::inc()
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
void BrakeCrane130::dec()
{
    int old_pos = handle_pos;

    handle_pos--;

    handle_pos = cut(handle_pos, min_pos, max_pos);

    if (handle_pos != old_pos)
        emit soundPlay("Kran_395_ruk");
}

GET_BRAKE_CRANE(BrakeCrane130)
