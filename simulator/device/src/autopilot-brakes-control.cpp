#include    <autopilot-brakes-control.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutopilotBrakeController::AutopilotBrakeController()
{
    connect(krm_handle_timer, &Timer::process,
            this, &AutopilotBrakeController::slotBrakeCraneHandle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::step(double t, double dt)
{
    krm_handle_timer->step(t, dt);

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::step_control(bool is_EPB_ON,
                                            double dv,
                                            bool is_motion_allowed,
                                            bool &lock_traction,
                                            bool &is_disable_release)
{
    if (is_EPB_ON)
    {
        stepEPB(dv, lock_traction, is_disable_release);
    }
    else
    {
        stepPB(dv, lock_traction, is_disable_release);
    }

    stepKVT(is_motion_allowed, is_disable_release);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepEPB(double dv,
                                       bool &lock_traction,
                                       bool &is_disable_release)
{
    // Максимальное превышение над программной скоростью
    const double dVminus = -0.5;
    // Максимальное снижение скорости относительно программной
    const double dVplus = 3.0;

    // Превышаем программую скорость
    if (dv < dVminus)
    {
        // Ступень торможения
        setBrakeCranePos(KRM_POS_Va);
        // Запрет тяги
        lock_traction = true;
    }

    // Скорость в допустимом коридоре
    if ( dv >= dVminus && dv <= dVplus)
    {
        // Ставим в перекрышу, при условии что в ТЦ минимут 1 кгс
        if (lock_traction && pBC >= 0.1)
        {
            setBrakeCranePos(KRM_POS_IV);
        }
    }

    // Скорость упала ниже коридора, нет запрета отпуска, запрещена тяга
    if (dv > dVplus && !is_disable_release && lock_traction)
    {
        // Отпускаем

        // Последняя ступень отпуска I положением
        if ( pBC < 0.1)
        {
            // если нет завышения в УР
            if (pEQ < p_charge + 0.02)
            {
                // Первое
                setBrakeCranePos(KRM_POS_I);
            }
            else // иначе - II положение
            {
                setBrakeCranePos(KRM_POS_II);
            }
        }
        else // и если не последняя ступень отпуска - отпускаем II положением
            setBrakeCranePos(KRM_POS_II);
    }

    // Не забываем рукоятку крана в первом положении!!!
    if (pEQ >= p_charge + 0.02 && bc_state.brake_crane_pos_ref == KRM_POS_I)
    {
        setBrakeCranePos(KRM_POS_II);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepPB(double dv,
                                      bool &lock_traction,
                                      bool &is_disable_release)
{
    // Максимальное превышение над программной скоростью
    const double dVminus = -0.5;
    // Максимальное снижение скорости относительно программной
    double dVplus = 3.0;

    if (dv < dVminus)
    {
        brakeStep(pEQ, p_charge, 0.04);
        lock_traction = true;
    }

    if (dv >= dVminus && dv <= dVplus)
    {
        if (lock_traction && pBC >= 0.1)
        {
            setBrakeCranePos(KRM_POS_IV);
        }
    }

    if (dv > dVplus && !is_disable_release && lock_traction)
    {
        brakeRelease(pEQ, p_charge, 0.0);
    }

    if (pEQ > p_charge)
    {
        setBrakeCranePos(KRM_POS_II);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepKVT(bool is_motion_allowed,
                                       bool &is_disable_release)
{
    // Если движение запрещено - зажимаем КВТ на полную
    if (!is_motion_allowed)
    {
        // ставим КВТ на полное торможение
        bc_state.loco_crane_pos_ref = 1.0;

        // Снимаем запрет отпуска
        is_disable_release = false;

        // отпускаем состав если кран в перекрыше
        if (bc_state.brake_crane_pos_ref == KRM_POS_III ||
            bc_state.brake_crane_pos_ref == KRM_POS_IV)
        {
            setBrakeCranePos(KRM_POS_I);
        }
    }
    else // Иначе - отпускаем
    {
        bc_state.loco_crane_pos_ref = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::setBrakeCranePos(int pos)
{
    if (!krm_handle_timer->isStarted())
    {
        // Переводим кран в новое положение с выдержкой по времени
        bc_state.brake_crane_pos_ref = pos;
        krm_handle_timer->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::brakeStep(double pEQ, double p_charge, double dp)
{
    if (pEQ > p_charge - dp)
    {
        setBrakeCranePos(KRM_POS_V);
    }
    else
    {
        setBrakeCranePos(KRM_POS_IV);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::brakeRelease(double pEQ, double p_charge, double dp_over)
{
    if (pEQ < p_charge + dp_over)
    {
        setBrakeCranePos(KRM_POS_I);
    }
    else
    {
        setBrakeCranePos(KRM_POS_II);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::slotBrakeCraneHandle()
{
    krm_handle_timer->stop();
}
