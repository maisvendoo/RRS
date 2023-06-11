#include    "kvt224.h"

#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane224::LocoCrane224(QObject *parent) : LocoCrane(parent)
  , V1(1e-4)
  , V3(3.5e-4)
  , p_switch(0.1)
  , min_pos(-0.05)
  , max_pos(1.0)
  , pos_duration(1.0)
  , dir(0)
  , step_pressures({0.0, 0.13, 0.20, 0.30, 0.40})
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);

    pos = cur_pos = 1.0;

    /*DebugLog *log = new DebugLog("kvt224.txt");
    connect(this, &LocoCrane224::DebugPrint, log, &DebugLog::DebugPring);*/
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
    double p_handle = k[1] * pf(pos);

    // Давление, задаваемое уравнительным органом крана
    double p_ur = max(p_handle, Y[P3_PRESSURE]);
    double dp_ur = k[2] * (p_ur - pBC);

    // Поток из питательной магистрали в магистраль тормозных цилиндров
    double Q_fl_bc = cut(dp_ur, 0.0, K[1]) * (pFL - pBC);

    // Разрядка магистрали тормозных цилиндров в атмосферу
    double Q_bc_atm = cut(-dp_ur, 0.0, K[2]) * pBC;

    // Работа повторительной схемы
    double dp_1 = pIL - Y[P1_PRESSURE];
    double u_switch = hs_n(dp_1 - p_switch);
    double u_release = hs_n(pos) + is_release;

    // Наполнение камеры над переключательным поршнем из импульсной магистрали
    double Q_il_1 = K[3] * dp_1 * u_switch;

    // Разрядка камеры над переключательным поршнем в атмосферу
    double Q_1_atm = K[4] * Y[P1_PRESSURE] * u_release;

    // Поток из камеры над переключательным поршнем в межпоршневое пространство
    double Q_1_3 = K[5] * (Y[P1_PRESSURE] - Y[P3_PRESSURE]);

    // Поток в питательную магистраль
    QFL = - Q_fl_bc;

    // Поток в магистраль тормозных цилиндров
    QBC = Q_fl_bc - Q_bc_atm;

    // Поток в импульсную магистраль
    QIL = - Q_il_1;

    // Поток в камеру над переключательным поршнем
    dYdt[P1_PRESSURE] = (Q_il_1 - Q_1_3 - Q_1_atm) / V1;

    // Поток в межпоршневое пространство и камеру 0.3 литра
    dYdt[P3_PRESSURE] = Q_1_3 / V3;
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
    cfg.getDouble(secName, "Vpz", V3);

    cfg.getDouble(secName, "ps", p_switch);

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
