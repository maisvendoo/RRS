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

    rb_timer->step(t, dt);

    Device::step(t, dt);
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
    v_ref = min(v_ref, calcAlsnSpeed(feedback->alsn_code));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_constr", v_constr);
    cfg.getDouble(secName, "BrakeAccel", a_brake);
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

    return sqrt(vt * vt + 2*a_brake*dist) * Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcAlsnSpeed(ALSN alsn_code)
{
    double v_lim = v_constr;

    switch (alsn_code)
    {
    case RED_YELLOW:

        v_lim = calcBrakeCurveSpeed(0.0, feedback->signal_dist);

        break;

    case YELLOW:

        v_lim = calcBrakeCurveSpeed(60.0, feedback->signal_dist);

        break;

    case NO_CODE:

        v_lim = 40.0; // ????

        break;

    case GREEN:

        break;
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotVigilanceControl()
{
    release_RB();
    rb_timer->stop();
}
