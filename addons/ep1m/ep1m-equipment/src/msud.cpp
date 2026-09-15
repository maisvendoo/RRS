#include    "msud.h"

#include    "defines.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MSUD::MSUD(QObject *parent) : Device(parent)
  , Uc(0.0)
  , Uc_min(40.0)
  , load_time(2.0)
  , load_timer(Q_NULLPTR)
  , is_fans_low_freq(true)
  , normalFreqTimer(new Timer(1.0, false))
  , lowFreqTimer(new Timer(1.0, false))
  , fansBustTimer(new Timer(2.5, true))
  , runOutTimer(new Timer(10.0, false))
  , fans_count(0)
  , fan_switch_timeout(80.0)
  , fan_bust_interval(2.5)
  , fan_runout_time(10.0)
  , I_fan_sw_min(480)
  , I_fan_sw_max(510)
  , is_pchf_ignored(false)
  , key_state_plus(false)
  , old_key_state_plus(false)
  , key_state_minus(false)
  , old_key_state_minus(false)
  , Ktp(0.0)
  , Kti(0.0)
  , Ktv(0.0)
  , dIa(0.0)
  , Vmax(140.0)
  , dV(0.0)
  , Ktvi(0.0)
  , Krp(0.0)
  , Kri(0.0)
  , Krv(0.0)
  , Krvi(0.0)
  , Ib_max(950.0)
  , If_max(845.0)
  , Vp(72.0)
{
    connect(normalFreqTimer, &Timer::process, this, &MSUD::slotNormalFreqTimer);

    connect(lowFreqTimer, &Timer::process, this, &MSUD::slotLowFreqTimer);

    connect(fansBustTimer, &Timer::process, this, &MSUD::slotFansBustTimer);

    connect(runOutTimer, &Timer::process, this, &MSUD::slotRunOutTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MSUD::~MSUD()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::init()
{
    load_timer = new Timer(load_time, false);
    connect(load_timer, &Timer::process, this, &MSUD::slotLoadTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::setPowerVoltage(double Uc)
{
    this->Uc = Uc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::ode_system(const state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)

    dYdt[0] = Kti * dIa;

    dYdt[1] = Ktvi * dV;

    dYdt[2] = Kri * dIa;

    dYdt[3] = Krvi * dV;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Y[0] = std::clamp(Y[0], -1.0, 1.0);
    Y[1] = std::clamp(Y[1], -1.0, 1.0);
    Y[2] = std::clamp(Y[2], -1.0, 1.0);
    Y[3] = std::clamp(Y[3], -Ib_max, Ib_max);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::stepDiscrete(double t, double dt)
{
    stateProcess(t, dt);

    load_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "FanSwitchTimeout", fan_switch_timeout);

    lowFreqTimer->setTimeout(fan_switch_timeout);
    normalFreqTimer->setTimeout(fan_switch_timeout);

    cfg.getDouble(secName, "FanBustInterval", fan_bust_interval);
    fansBustTimer->setTimeout(fan_bust_interval);

    cfg.getDouble(secName, "FanRunoutTime", fan_runout_time);
    runOutTimer->setTimeout(fan_runout_time);

    cfg.getDouble(secName, "FanSwitchCurrentMin", I_fan_sw_min);

    cfg.getDouble(secName, "FanSwitchCurrentMax", I_fan_sw_max);

    cfg.getBool(secName, "Ignore_Off_PCHF", is_pchf_ignored);

    cfg.getDouble(secName, "TC_min_press", msud_output.TC_min_press);

    QDomNode secNode = cfg.getFirstSection("Zone");

    while (!secNode.isNull())
    {
        zone_t zone;

        cfg.getDouble(secNode, "Umin", zone.Umin);
        cfg.getDouble(secNode, "Umax", zone.Umax);

        vip_zone.push_back(zone);

        secNode = cfg.getNextSection();
    }

    cfg.getDouble(secName, "Ktp", Ktp);
    cfg.getDouble(secName, "Kti", Kti);
    cfg.getDouble(secName, "Ktv", Ktv);
    cfg.getDouble(secName, "Vmax", msud_output.Vmax);
    cfg.getDouble(secName, "Ktvi", Ktvi);

    cfg.getDouble(secName, "Krp", Krp);
    cfg.getDouble(secName, "Kri", Kri);
    cfg.getDouble(secName, "Krv", Krv);
    cfg.getDouble(secName, "Krvi", Krvi);

    cfg.getDouble(secName, "Ib_max", msud_output.Ib_max);
    cfg.getDouble(secName, "If_max", msud_output.If_max);
    cfg.getDouble(secName, "Ia_max", msud_output.Ia_max);
    cfg.getDouble(secName, "Vp", Vp);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    old_key_state_plus = key_state_plus;
    key_state_plus = getKeyState(pressed_keys, KEY_Equals);

    old_key_state_minus = key_state_minus;
    key_state_minus = getKeyState(pressed_keys, KEY_Minus);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::stateProcess(double t, double dt)
{
    switch (msud_output.state)
    {
    case MSUD_OFF:
        {
            // Переход в режим общего сброса при включении питания
            if ( Uc >= Uc_min )
            {
                msud_output.state = MSUD_RESET;
            }

            break;
        }

    case MSUD_RESET:
        {
            // Общий сброс
            reset();
            // Ожидание загрузки
            load_timer->start();

            break;
        }

    case MSUD_READY:
        {
            // Основной цикл работы МСУД
            main_loop(t, dt);
            break;
        }
    }

    // Если напряжение питания упало ниже минимально допустимого
    // МСУД переходит в выключенное состояние
    if (Uc < Uc_min)
        msud_output.state = MSUD_OFF;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::reset()
{
    // Снимаем все управляющие сигналы
    msud_output = msud_output_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::main_loop(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Подаем питание на реле KV23 (временно, потом будем считать отключения ГВ)
    msud_output.kv23_On = true;

    // Подаем питание на реле KV14
    msud_output.kv14_On = true;

    motor_fans_control(t, dt);

    brake_cylinders_pressure_control(t, dt);

    traction_control(t, dt);

    recuperation_control(t, dt);

    emergancy_brake_boost();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::motor_fans_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Тики таймеров
    normalFreqTimer->step(t, dt);
    lowFreqTimer->step(t, dt);
    fansBustTimer->step(t, dt);
    runOutTimer->step(t, dt);

    // Признак работы без ПЧФ, только на нормальной частоте
    bool is_Norm_Freq_Only = (!msud_input.is_PCHF_On) || (is_pchf_ignored);

    // Проверяем, запущены ли венты
    bool fans_run = true;

    for (size_t i = 0; i < msud_input.mv_state.size(); ++i)
        fans_run = fans_run && msud_input.mv_state[i];


    if (!is_Norm_Freq_Only)
    {
        // Работаем при включенном ПЧФ
        bool is_low_freq = msud_output.mv_freq_low[MV1] ||
                msud_output.mv_freq_low[MV2] ||
                msud_output.mv_freq_low[MV3];

        bool is_norm_freq = msud_output.mv_freq_norm[MV1] ||
                msud_output.mv_freq_norm[MV2] ||
                msud_output.mv_freq_norm[MV3];

        msud_output.is_MV_low_freq = fans_run && is_low_freq;

        if (fans_run)
        {
            if (msud_input.Ia[TRAC_MOTOR1] <= I_fan_sw_min)
            {
                if (!lowFreqTimer->isStarted() && !is_low_freq)
                    lowFreqTimer->start();
            }

            if (msud_input.Ia[TRAC_MOTOR1] >= I_fan_sw_max)
            {
                if (!normalFreqTimer->isStarted() && !is_norm_freq)
                    normalFreqTimer->start();
            }
        }
        else
        {
            for (size_t i = 0; i < msud_output.mv_freq_low.size(); ++i)
            {
                msud_output.mv_freq_low[i] = true;
                msud_output.mv_freq_norm[i] = false;
            }
        }
    }
    else
    {
        msud_output.is_MV_low_freq = false;
        lowFreqTimer->stop();
        normalFreqTimer->stop();
        runOutTimer->stop();

        if (!fansBustTimer->isStarted())
            fansBustTimer->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::brake_cylinders_pressure_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    bool is_TC_full = false;

    for (size_t i = 0; i < msud_output.TC_full.size(); ++i)
    {
        msud_output.TC_full[i] = msud_input.TC_press[i] > msud_output.TC_min_press;
        is_TC_full |= msud_output.TC_full[i];
    }

    if (is_TC_full)
    {
        msud_output.TC_status = MSUD_STATUS_BLINK;
    }
    else
    {
        msud_output.TC_status = MSUD_STATUS_OFF;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::traction_control(double t, double dt)
{
    if (msud_input.is_auto_reg)
    {
        if ( (msud_input.is_traction) && (!msud_input.is_brake) )
        {
            auto_traction_control(t, dt);
            field_weak_control(t, dt);
        }
        else
        {
            if (!msud_input.is_brake)
                reset_traction_control();
        }
    }
    else
    {
        if (msud_input.is_traction)
        {
            manual_traction_control(t, dt);
            field_weak_control(t, dt);
        }
        else
        {
            if (!msud_input.is_brake)
                reset_traction_control();
        }
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::manual_traction_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Максимальное напряжение на выходе ВИП
    double Ud_max = (*(vip_zone.end()-1)).Umax;

    double Ud = msud_input.km_trac_level * Ud_max;

    vip_control(Ud);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::auto_traction_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Расчитываем абсолютное значение заданной скорости (км/ч)
    double V_ref = msud_input.km_ref_velocity_level * msud_output.Vmax;

    // Рассчитываем максимальный заданный ток якоря
    double Ia_ref_max = msud_input.km_trac_level * msud_output.Ia_max;

    // Вычисляем ошибку по скорости (км/ч)
    double dV = V_ref - msud_input.V_cur;

    // Определяем требуемый якоря ток в тяге
    double Ia_ref = Ktv * dV + getY(1);

    // Ограничиваем ток величиной, заданной с главного вала КМ
    Ia_ref = std::clamp(Ia_ref, 0.0, Ia_ref_max);

    // Расчет ошибки по току
    dIa = Ia_ref - msud_input.Ia[TRAC_MOTOR1];

    // Максимальное напряжение, которое способен выдать ВИП
    double U_max = (*(vip_zone.end() - 1)).Umax;

    // Расчитываем относительную величину напряжения ВИТ
    double u = Ktp * dIa + getY(0);

    u = std::clamp(u, 0.0, 1.0);

    // Рассчитываем абсолютную величину напряжения ВИП
    double Ud = U_max * u;

    vip_control(Ud);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::reset_traction_control()
{
    msud_output.alpha = 90.0;
    msud_output.zone_num = ZONE1 + 1;
    msud_output.vip_voltage_level = 0.0;
    msud_output.field_weak_step = STEP0;
    std::fill(msud_output.op.begin(), msud_output.op.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t MSUD::select_traction_VIP_Zone(double Ud)
{
    size_t zone_idx = 0;

    for (size_t i = 0; i < vip_zone.size(); ++i)
    {
        if ( (Ud >= vip_zone[i].Umin) && (Ud < vip_zone[i].Umax) )
        {
            zone_idx = i;
            return zone_idx;

        }
    }

    if (Ud >= (*(vip_zone.end() - 1)).Umax)
        zone_idx = vip_zone.size() - 1;

    if (Ud < (*(vip_zone.begin())).Umin)
        zone_idx = 0;

    return zone_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::vip_control(double Ud)
{
    size_t zone_idx = select_traction_VIP_Zone(Ud);
    msud_output.zone_num = zone_idx + 1;
    msud_output.zone_num = std::clamp(msud_output.zone_num,
                                      static_cast<std::uint8_t>(1),
                                      static_cast<std::uint8_t>(4));

    double Umin = vip_zone[zone_idx].Umin;
    double Umax = vip_zone[zone_idx].Umax;

    double cos_alpha = (Ud - Umin) / (Umax - Umin);

    msud_output.alpha = acos(cos_alpha) * 180.0 / Physics::PI;
    msud_output.vip_voltage_level = static_cast<double>(zone_idx) + cos_alpha;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::field_weak_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Работа только при полностью открытой 4-й зоне ВИП
    if (static_cast<int>(msud_output.vip_voltage_level) != ZONE4 + 1)
    {
        std::fill(msud_output.op.begin(), msud_output.op.end(), false);
        msud_output.field_weak_step = STEP0;
        return;
    }

    if (key_state_plus && !old_key_state_plus)
    {
        msud_output.field_weak_step++;
    }

    if (key_state_minus && !old_key_state_minus)
    {
        msud_output.field_weak_step--;
    }

    msud_output.field_weak_step = std::clamp(msud_output.field_weak_step,
                                             static_cast<std::uint8_t>(STEP0),
                                             static_cast<std::uint8_t>(STEP3));

    for (size_t i = 0; i < msud_output.op.size(); ++i)
    {
        if (i <= msud_output.field_weak_step)
            msud_output.op[i] = true;
        else
            msud_output.op[i] = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::recuperation_control(double t, double dt)
{
    if (msud_input.is_auto_reg)
    {
        if (msud_input.is_brake)
        {
            auto_recuperation_control(t, dt);
        }
        else
        {
            if (!msud_input.is_traction)
                reset_recuperaion_control();
        }
    }
    else
    {
        if (msud_input.is_brake)
            manual_recuperation_control(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::auto_recuperation_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    double V_ref = msud_input.km_ref_velocity_level * Vmax;

    dV = V_ref - msud_input.V_cur;

    double I_ref = Krv * dV + getY(3);

    I_ref = std::clamp(I_ref, - msud_output.Ib_max * msud_input.km_brake_level, 0.0);

    brake_current_regulator(I_ref);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::manual_recuperation_control(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    //brake_current_regulator(-msud_output.Ib_max * msud_input.km_brake_level);
    double Ia_ref = -msud_output.Ib_max * msud_input.km_brake_level;

    // Вычисляем ошибку по току якоря
    double Ia = msud_input.Ia[TRAC_MOTOR1];

    dIa = Ia_ref - Ia;

    // Максимальное напряжение, которое способен выдать ВИП
    double U_max = (*(vip_zone.end() - 1)).Umax;

    double u = Krp * dIa + getY(2);

    u = std::clamp(u, -1.0, 0.0);

    double Ud = U_max * (1 + u) * hs_n(Ia);

    vip_control(Ud);

    msud_output.field_level = msud_input.km_ref_velocity_level;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::reset_recuperaion_control()
{
    msud_output.alpha = 90.0;
    msud_output.zone_num = ZONE1 + 1;
    msud_output.vip_voltage_level = 0.0;
    msud_output.field_level = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::brake_current_regulator(double Ia_ref)
{
    // Вычисляем ошибку по току якоря
    double Ia = msud_input.Ia[TRAC_MOTOR1];

    dIa = Ia_ref - Ia;

    // Максимальное напряжение, которое способен выдать ВИП
    double U_max = (*(vip_zone.end() - 1)).Umax;

    double u = Krp * dIa + getY(2);

    u = std::clamp(u, -1.0, 0.0);

    double Ud = U_max * (1 + u) * hs_n(Ia);

    vip_control(Ud);

    double s = pow(Vp / (msud_input.V_cur + 0.1), 2);

    msud_output.field_level = s * qAbs(u);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::emergancy_brake_boost()
{
    if ( (msud_input.V_cur > 60) && msud_input.is_emergency_brake )
    {
        msud_output.is_not_brake_boost = false;
    }
    else
    {
        msud_output.is_not_brake_boost = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::slotLoadTimer()
{
    load_timer->stop();
    msud_output.state = MSUD_READY;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::slotNormalFreqTimer()
{
    if (!fansBustTimer->isStarted())
    {
        fansBustTimer->start();
        fans_count = MV1;
        normalFreqTimer->stop();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::slotLowFreqTimer()
{
    for (size_t i = 0; i < msud_output.mv_freq_norm.size(); ++i)
    {
        msud_output.mv_freq_norm[i] = false;
    }

    if (!runOutTimer->isStarted())
        runOutTimer->start();

    lowFreqTimer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::slotFansBustTimer()
{
    msud_output.mv_freq_low[fans_count] = false;
    msud_output.mv_freq_norm[fans_count] = true;
    fans_count++;

    if (fans_count == msud_output.mv_freq_norm.size())
    {
        fans_count = MV1;
        fansBustTimer->stop();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUD::slotRunOutTimer()
{
    for (size_t i = 0; i < msud_output.mv_freq_low.size(); ++i)
    {
        msud_output.mv_freq_low[i] = true;
        msud_output.mv_freq_norm[i] = false;
    }

    runOutTimer->stop();
}
