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
  , pos_num(0)
  , positions({0.0, 0.325, 0.5, 0.752, 1.0})
  , step_pressures({0.0, 0.13, 0.20, 0.30, 0.40})
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);

    pos = max_pos;

    sounds[CHANGE_POS_SOUND] = sound_state_t();
    sounds[BC_FILL_FLOW_SOUND] = sound_state_t(true, 0.0f, 1.0f);
    sounds[BC_DRAIN_FLOW_SOUND] = sound_state_t(true, 0.0f, 1.0f);
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
void LocoCrane254::setHandlePosition(double position)
{
    pos = cut(position, min_pos, max_pos);
    int new_pos_num = getPositionNumber();
    if (pos_num == new_pos_num)
        return;
    pos_num = new_pos_num;
    sounds[CHANGE_POS_SOUND].play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getHandlePosition() const
{
    return cut(pos, min_pos, max_pos);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getHandleShift() const
{
    return cut(pos, 0.0, max_pos);
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
    double Q_fl_bc = K[1] * cut(dp_ur, 0.0, 1.0) * pf(pFL - pBC);

    // Разрядка магистрали тормозных цилиндров в атмосферу
    double Q_bc_atm = K[2] * cut(-dp_ur, 0.0, 1.0) * pBC;

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
void LocoCrane254::stepSound()
{
    sounds[BC_FILL_FLOW_SOUND].volume = pf(QBC) * 500.0f;
    sounds[BC_DRAIN_FLOW_SOUND].volume = nf(QBC) * 500.0f;
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

    double new_pos = pos;

    // Непрерывное движение ручки в сторону отпуска
    if (getKeyState(KEY_Leftbracket))
        new_pos = max(min_pos, pos - pos_duration * dt);
    else
    {
        // Возврат из отпускного положения
        if ( (pos < 0.0) && (!static_cast<bool>(is_release)) )
            new_pos = 0.0;
        else
        {
            // Непрерывное движение ручки в сторону торможения
            if (getKeyState(KEY_Rightbracket))
                new_pos = min(max_pos, pos + pos_duration * dt);
        }
    }

    // Дискретное движение от кнопок
    double max_step = *(step_pressures.end() - 1);

    if (isAlt())
    {
        if (getKeyState(KEY_8))
        {
            new_pos = step_pressures[0] / max_step;
        }

        if (getKeyState(KEY_9))
        {
            new_pos = step_pressures[1] / max_step;
        }

        if (getKeyState(KEY_0))
        {
            new_pos = step_pressures[2] / max_step;
        }

        if (getKeyState(KEY_Minus))
        {
            new_pos = step_pressures[3] / max_step;
        }

        if (getKeyState(KEY_Equals))
        {
            new_pos = step_pressures[4] / max_step;
        }
    }

    setHandlePosition(new_pos);

    stepSound();
}

//------------------------------------------------------------------------------
// Позиция крана по канавкам (только для звуков)
//------------------------------------------------------------------------------
int LocoCrane254::getPositionNumber() const
{
    int pos_n = -1;

    if (pos <= 0.0)
        return 0;

    if (pos == 1.0)
        return 4;

    for (uint i = 0; i < (positions.size() - 1); ++i)
    {
        if (pos > positions[i] && pos <= positions[i+1])
            pos_n = static_cast<int>(i);
    }

    return pos_n;
}

GET_LOCO_CRANE(LocoCrane254)
