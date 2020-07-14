#include    "kvt224.h"

#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane224::LocoCrane224(QObject *parent) : LocoCrane(parent)
  , V1(1e-4)
  , V2(1e-4)
  , Vpz(3e-4)
  , delta_p(0.05)
  , ps(0.1)
  , min_pos(-0.05)
  , max_pos(1.0)
  , pos_duration(1.0)
  , dir(0)
  , step_pressures({0.0, 0.13, 0.20, 0.30, 0.40})
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);

    pos = cur_pos = 1.0;

    /*DebugLog *log = new DebugLog("kvt254.txt");
    connect(this, &LocoCrane254::DebugPrint, log, &DebugLog::DebugPring);*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane224::~LocoCrane224()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane224::getHandlePosition() const
{
    return cur_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane224::getHandleShift() const
{
    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane224::getAirDistribPressure() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane224::init(double pTM, double pFL)
{
    Q_UNUSED(pTM)
    Q_UNUSED(pFL)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane224::ode_system(const state_vector_t &Y,
                              state_vector_t &dYdt,
                              double t)
{
    Q_UNUSED(t)

    // Давление, задаваемое поворотом рукоятки
    double p_handle = K[3] * pf(cur_pos);

    double u1 = hs_p(p_handle - Y[2]);

    double u2 = hs_n(p_handle - Y[2]);

    // Давление, задаваемое уравнительным органом крана
    double pz = p_handle * u1 + Y[2] * u2;

    double dp = pz - pBC;

    double u3 = cut(pf(k[1] * dp), 0.0, 1.0);

    double u4 = cut(nf(k[1] * dp), 0.0, 1.0);

    // Поток воздуха в ТЦ
    Qbc = K[1] * (pFL - pBC) * u3 - K[2] * pBC * u4;

    // Работа повторительной схемы

    double dp12 =  Y[0] - Y[1];

    double u5 = hs_n(dp12 - ps);

    double u6 = hs_n(pos) + is_release;

    double Qpz = K[7] * (Y[1] - Y[2]);

    double Q12 = K[5] * dp12 * u5;

    double Q1 = K[4] * Qvr;

    double Q2 = Q12 - Qpz - K[6] * Y[1] * u6;

    dYdt[0] = Q1 / V1; // p1

    dYdt[1] = Q2 / V2; // p2

    dYdt[2] = Qpz / Vpz; // p_pz
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane224::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 1; i < k.size(); ++i)
    {
        QString coeff = QString("k%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }

    cfg.getDouble(secName, "V1", V1);
    cfg.getDouble(secName, "V2", V2);
    cfg.getDouble(secName, "Vpz", Vpz);

    cfg.getDouble(secName, "delta_p", delta_p);

    cfg.getDouble(secName, "ps", ps);

    QString tmp = "";

    cfg.getString(secName, "StepPressures", tmp);

    std::istringstream ss(tmp.toStdString());

    for (size_t i = 0; i < NUM_STEPS; ++i)
    {
        double step_press = 0.0;
        ss >> step_press;

        step_pressures[i] = step_press;
    }

    for (size_t i = 0; i < step_pressures.size(); ++i)
    {
        fixed_pos[i] = static_cast<double>(i) / (NUM_STEPS - 1);
    }

    cfg.getDouble(secName, "MinPos", min_pos);
    cfg.getDouble(secName, "MaxPos", max_pos);
    cfg.getDouble(secName, "PosDuration", pos_duration);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane224::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)

    // Непрерывное движение ручки
    if (getKeyState(KEY_Leftbracket))
        dir = -1;
    else
    {
        if (getKeyState(KEY_Rightbracket))
            dir = 1;
        else
            dir = 0;
    }

    pos += dir * pos_duration * dt;

    pos = cut(pos, min_pos, max_pos);

    for (size_t i = 0; i < fixed_pos.size() - 1; ++i)
    {
        if ( (pos >= fixed_pos[i]) && (pos < fixed_pos[i+1]) )
            cur_pos = fixed_pos[i];
    }

    if (pos > 0.99 * *(fixed_pos.end() - 1))
        cur_pos = *(fixed_pos.end() - 1);

    if (isAlt())
    {
        if (getKeyState(KEY_8))
        {
            cur_pos = fixed_pos[0];
        }

        if (getKeyState(KEY_9))
        {
            cur_pos = fixed_pos[1];
        }

        if (getKeyState(KEY_0))
        {
            cur_pos = fixed_pos[2];
        }

        if (getKeyState(KEY_Minus))
        {
            cur_pos = fixed_pos[3];
        }

        if (getKeyState(KEY_Equals))
        {
            cur_pos = fixed_pos[4];
        }
    }
}

GET_LOCO_CRANE(LocoCrane224)
