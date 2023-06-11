#include    "kvt254.h"

#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane254::LocoCrane254(QObject *parent) : LocoCrane(parent)
  , V1(1e-4)
  , V3(3.5e-4)
  , p_switch(0.1)
  , min_pos(-0.05)
  , max_pos(1.0)
  , pos_duration(1.0)
  , volume(0)
  , p_volume(0)
  , dir(0)
  , pos_num(0)
  , isStop(false)
  , positions({0.0, 0.325, 0.5, 0.752, 1.0})
  , step_pressures({0.0, 0.13, 0.20, 0.30, 0.40})
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);

    pos = 1.0;

    /*DebugLog *log = new DebugLog("kvt254.txt");
    connect(this, &LocoCrane254::DebugPrint, log, &DebugLog::DebugPring);*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane254::~LocoCrane254()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getHandlePosition() const
{
    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getHandleShift() const
{
    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::init(double pBP, double pFL)
{
    Q_UNUSED(pBP)
    Q_UNUSED(pFL)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::ode_system(const state_vector_t &Y,
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

    stepSound();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::stepSound()
{
    p_volume = volume;

    // 250000 поправочный коэффициент для перевода 1кг/cм^3 в 1кг/м^3
    // Для звуков взято 400000 с малым запасом
    volume = abs(QBC) * 400000;

    if (volume > 30)
    {
        if (QBC > 0)
        {
            if (p_volume <= 30)
            {
                emit soundPlay("254_vpusk");
            }
            emit soundSetVolume("254_vypusk", 0);
            emit soundSetVolume("254_vpusk", volume);
        }

        if (QBC < 0)
        {
            if (p_volume <= 30)
            {
                emit soundPlay("254_vypusk");
            }
            emit soundSetVolume("254_vpusk", 0);
            emit soundSetVolume("254_vypusk", volume);
        }
        isStop = false;
    }
    else
    {
        if (!isStop)
        {
            emit soundStop("254_vpusk");
            emit soundStop("254_vypusk");
            isStop = true;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::load_config(CfgReader &cfg)
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

    cfg.getDouble(secName, "MinPos", min_pos);
    cfg.getDouble(secName, "MaxPos", max_pos);
    cfg.getDouble(secName, "PosDuration", pos_duration);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::stepKeysControl(double t, double dt)
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

        // Возврат из отпускного положения
        if (pos < 0.0)
           pos = 0.0;
    }

    int old_pos_n = pos_num;

    pos += dir * pos_duration * dt;

    pos = cut(pos, min_pos, max_pos);

    // Дискретное движение от кнопок
    double max_step = *(step_pressures.end() - 1);

    if (isAlt())
    {
        if (getKeyState(KEY_8))
        {
            pos = step_pressures[0] / max_step;
        }

        if (getKeyState(KEY_9))
        {
            pos = step_pressures[1] / max_step;
        }

        if (getKeyState(KEY_0))
        {
            pos = step_pressures[2] / max_step;
        }

        if (getKeyState(KEY_Minus))
        {
            pos = step_pressures[3] / max_step;
        }

        if (getKeyState(KEY_Equals))
        {
            pos = step_pressures[4] / max_step;
        }
    }

    pos_num = getPositionNumber();

    if (pos_num != old_pos_n && pos_num != -1)
        emit soundPlay("254-chelk");
}

//------------------------------------------------------------------------------
// Позиция крана по канавкам (только для звуков)
//------------------------------------------------------------------------------
int LocoCrane254::getPositionNumber() const
{
    int pos_n = -1;

    if (pos == 0.0)
        return 0;

    if (pos == 1.0)
        return 4;

    for (uint i = 0; i < positions.size() - (dir == -1 ? 2 : 1); ++i)
    {
        if (pos >= positions[i] && pos <= positions[i+1])
            pos_n = static_cast<int>(i);
    }

    return pos_n;
}

GET_LOCO_CRANE(LocoCrane254)
