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
    // (добавиться логика определения лимита по кивой торможения и по времени хода!!!)
    v_ref = min(feedback->v_lim, v_constr);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_constr", v_constr);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotVigilanceControl()
{
    release_RB();
    rb_timer->stop();
}
