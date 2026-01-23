#include    <autopilot.h>
#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Autopilot *loadAutopilot(QString lib_path)
{
    Autopilot *autopilot = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetAutopilot getAutopilot = reinterpret_cast<GetAutopilot>(lib.resolve("getAutopilot"));

        if (getAutopilot)
        {
            autopilot = getAutopilot();
        }
    }

    return autopilot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::step(double t, double dt)
{
    velocity_control(t, dt);

    vigilance_control(t, dt);

    accel_meter->setVelocity(feedback->v_cur);
    accel_meter->step(t, dt);

    rb_timer->step(t, dt);

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Autopilot::getDbgMsg()
{
    double v_p = calcPredictVelocity(feedback->v_cur, dist_target, accel_meter->value());

    return QString(" | ВКЛЮЧЕНО АВТОВЕДЕНИЕ | Vзад.: %1 км/ч| Уск.: %2 м/с2| Зад. уск.: %3 м/с2 | Прогноз Vцел.: %4 км/ч")
        .arg(v_ref, 4, 'f', 1)
        .arg(accel_meter->value(), 6, 'f', 2)
        .arg(-a_brake, 6, 'f', 2)
        .arg(v_p, 4, 'f', 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::vigilance_control(double t, double dt)
{
    if (feedback == nullptr)
    {
        return;
    }

    // Есть сигнал контроля бдительности
    if (feedback->is_vigilance_control)
    {
        if (!rb_timer->isStarted())
        {
            // Жмем РБ
            press_RB();
            // Запускаем таймер
            rb_timer->start();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::velocity_control(double t, double dt)
{
    if (feedback == nullptr)
        return;

    // Выбираем минимум между текущим ограничением и конструкционной скоростью
    v_ref = min(calcCurrentSpeedLimit(t, dt), v_constr);

    // Рассчитываем скорость по тормозной кривой до следующего ограничения
    // (если оно больше, ну и пусть :))) )
    v_ref = min(v_ref, calcBrakeCurveSpeed(feedback->v_lim_next, feedback->limit_dist));

    // Расчитываем скорость по тормозной кривой до ближайшего сигнала
    v_ref = min(v_ref, calcAlsnSpeed(feedback->alsn_code, feedback->signal_dist, v_target));

    // Минимальная целевая скорость (для предсказания тормозного пути)
    v_target = min(feedback->v_lim_next, v_target);

    if (feedback->v_lim_next < feedback->v_lim)
        dist_target = min(feedback->v_lim_next, feedback->signal_dist);
    else
        dist_target = feedback->signal_dist;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_constr", v_constr);
    cfg.getDouble(secName, "BrakeAccel", a_brake_ref);
    cfg.getDouble(secName, "RefLength", ref_length);
    cfg.getDouble(secName, "RefMass", ref_mass);

    ref_mass = ref_mass * 1000.0;

    cfg.getDouble(secName, "LeadDistance_RY", lead_dist_RY);
    cfg.getDouble(secName, "LeadDistance_Y", lead_dist_Y);
    cfg.getDouble(secName, "SpeedLimit_RY", v_lim_RY);
    cfg.getDouble(secName, "SpeedDisableRelease", v_disable_release);
    cfg.getDouble(secName, "dVTractionOff", dV_traction_off);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcCurrentSpeedLimit(double t, double dt)
{
    double v_lim = feedback->v_lim;

    if (feedback->v_lim < feedback->v_lim_next)
    {
        // Запоминаем текущее, более строгое ограничение
        prev_v_lim = feedback->v_lim;
        // ЗАпоминаем длину хвоста, который надо затянуть
        tail_len = train_length;
    }
    else
    {
        // Затянут весь хвост
        if (tail_len <= 0)
        {
            v_lim = feedback->v_lim;
        }
        else // хвост не затянут, едем по старому ограничению
        {
            v_lim = prev_v_lim;
            tail_len -= feedback->v_cur *dt / Physics::kmh;
        }
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcBrakeCurveSpeed(double v_target, double dist)
{
    double vt = v_target / Physics::kmh;

    a_brake = a_brake_ref * ref_mass * train_length / train_mass / ref_length;

    return sqrt(vt * vt + pf(2 * a_brake * dist)) * Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcAlsnSpeed(ALSN alsn_code, double signal_dist, double &v_target)
{
    double v_lim = v_constr;

    is_disable_release = false;

    switch (alsn_code)
    {
    case RED_YELLOW:

        v_target = 0.0;

        v_lim = cut(calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_RY), 0.0, v_lim_RY);

        // Запрещаем отпускать тормоза - остановка
        if (feedback->v_cur <= v_disable_release)
        {
            is_disable_release = true;            
        }

        // Если запрещен отпуск и мы остановились - запрещаем движение
        if ( (feedback->v_cur <= 1.0 || signal_dist <= lead_dist_RY) && is_disable_release)
        {
            is_motion_allowed = false;
        }

        break;

    case YELLOW:

        v_target = v_lim_RY;

        v_lim = calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_Y);
        is_motion_allowed = true;

        break;

    case NO_CODE:

        // Тут, по идее, будет происходить торможение, а проверка бдительности
        // не даст сорвать ЭПК
        v_lim = 40.0; // ????

        is_motion_allowed = true;

        break;

    case GREEN:

        is_motion_allowed = true;

        break;
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcPredictVelocity(double v_cur, double dist, double accel)
{
    double v0 = v_cur / Physics::kmh;

    double tmp = v0 * v0 + 2 * accel *dist;

    double v_p = 0.0;

    if (tmp < 0)
    {
        return v_p;
    }
    else
    {
        v_p = sqrt(tmp) * Physics::kmh;
    }

    for (size_t i = 0; i < v_filter.size() - 1; ++i)
    {
        v_filter[i] = v_filter[i + 1];
    }

    *(v_filter.end() - 1) = v_p;

    double sum = 0.0;

    for (auto v : v_filter)
    {
        sum += v;
    }

    return sum / v_filter.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSetBrakeAccel(double a_brake)
{
    this->a_brake = a_brake;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotVigilanceControl()
{
    release_RB();
    rb_timer->stop();
}
