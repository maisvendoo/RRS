#include    <autopilot-brakes-control.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutopilotBrakeController::AutopilotBrakeController()
{
    connect(krm_handle_timer, &Timer::process,
            this, &AutopilotBrakeController::slotBrakeCraneHandle);

    connect(brakePB_timer, &Timer::process,
            this, &AutopilotBrakeController::slotBrakePBdelay);

    connect(brakeEPB_timer, &Timer::process,
            this, &AutopilotBrakeController::slotBrakeEPBdelay);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::step(double t, double dt)
{
    krm_handle_timer->step(t, dt);
    brakePB_timer->step(t, dt);
    brakeEPB_timer->step(t, dt);

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
    // Управляем КВТ
    stepKVT(is_motion_allowed, is_disable_release);

    // Если включен ЭПТ
    if (is_EPB_ON)
    {
        // Управляем им
        stepEPB(dv, lock_traction, is_disable_release, is_motion_allowed);
    }
    else // иначе - пичаль-пичалька
    {
        // едем на превматике
        stepPB(dv, is_motion_allowed, lock_traction, is_disable_release);
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "dVminusEPB", dVminusEPB);
    cfg.getDouble(secName, "dVplusEPB", dVplusEPB);
    cfg.getDouble(secName, "dVminusPB", dVminusPB);
    cfg.getDouble(secName, "dVplusPB", dVplusPB);
    cfg.getDouble(secName, "dpEPB_over", dpEPB_over);
    cfg.getDouble(secName, "dpPB_over", dpPB_over);
    cfg.getDouble(secName, "dpFirstStep", dp_first_step);
    cfg.getDouble(secName, "dpOtherStep", dp_other_step);

    cfg.getDouble(secName, "HoldTimeout", hold_timeout);

    brakePB_timer->setTimeout(hold_timeout);

    cfg.getDouble(secName, "pBC_EPB", pBC_EPB);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepEPB(double dv,
                                       bool &lock_traction,
                                       bool &is_disable_release,
                                       bool is_motion_allowed)
{
    // Максимальное превышение над программной скоростью
    const double dVminus = -dVminusEPB;
    // Максимальное снижение скорости относительно программной
    const double dVplus = dVplusEPB;

    if (bc_state.brake_crane_pos_ref == KRM_POS_I ||
        bc_state.brake_crane_pos_ref == KRM_POS_II)
    {
        num_EPB_steps = 0;
    }

    // В тяге - держим рукоятку в поездном положении и нехер её дергать
    if (!lock_traction)
    {
        bc_state.brake_crane_pos_ref = KRM_POS_II;
    }

    // Превышаем программую скорость
    if (dv < dVminus)
    {
        // Ступень торможения
        brakeStepEPB(pBC, pBC_EPB);
        // Запрет тяги
        lock_traction = true;
    }

    // Скорость упала ниже коридора, нет запрета отпуска, запрещена тяга
    if (dv > dVplus && !is_disable_release && lock_traction && is_motion_allowed)
    {
        // Отпускаем
        brakeReleaseEPB(pEQ, pBC, p_charge, dpEPB_over);
    }

    // При давлении в УР выше зарядного
    if (pEQ > p_charge + dpEPB_over)
    {
        // Ставим кран во второе положение
        setBrakeCranePos(KRM_POS_II);
    }

    // Cтупень безусловно, если не задействован КВТ
    if (!is_motion_allowed)
    {
        if (bc_state.loco_crane_pos_ref < 0.01)
        {
            brakeStepEPB(pBC, pBC_EPB);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepPB(double dv,
                                      bool is_motion_allowed,
                                      bool &lock_traction,
                                      bool &is_disable_release)
{
    // Максимальное превышение над программной скоростью
    const double dVminus = -dVminusPB;
    // Максимальное снижение скорости относительно программной
    double dVplus = dVplusPB;

    // Обнуляем число дополнительных ступеней, если кран в отпускном положении
    if (bc_state.brake_crane_pos_ref == KRM_POS_I ||
        bc_state.brake_crane_pos_ref == KRM_POS_II)
    {
        num_PB_steps = 0;
    }

    // Первысили кривую снижения скорости
    if (dv < dVminus)
    {
        // Даем ступень
        brakeStep(pEQ, p_charge, dp_first_step);
        // Запрет тяги
        lock_traction = true;
    }

    // Опустились достаточно низко под кривую снижения скорости, и если
    // отпуск не запрещен
    if (dv > dVplus && !is_disable_release)
    {
        // полный отпуск
        brakeReleasePB(pEQ, p_charge, dpPB_over);
    }

    // При давлении в УР выше зарядного
    if (pEQ > p_charge + dpPB_over)
    {
        // Ставим кран во второе положение
        setBrakeCranePos(KRM_POS_II);
    }

    // Запрет отпуска - ступень безусловно, если не задействован КВТ
    if (!is_motion_allowed)
    {
        if (bc_state.loco_crane_pos_ref < 0.01)
        {
            brakeStep(pEQ, p_charge, dp_first_step);
        }
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

        // отпускаем состав если кран не в поездном положении
        if (bc_state.brake_crane_pos_ref != KRM_POS_II)
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
    if (pEQ > p_charge - dp - num_PB_steps * 0.02)
    {
        if (!brakePB_timer->isStarted())
        {
            // Даем разрядку до заданной глубины
            bc_state.brake_crane_pos_ref = KRM_POS_V;
            // Запускаем таймер выдержки времени на данной ступени
            brakePB_timer->start();
        }
    }
    else
    {
        // Ставим в перекрышу при достаточной глубине разрядки
        setBrakeCranePos(KRM_POS_IV);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::brakeReleasePB(double pEQ, double p_charge, double dp_over)
{
    // Отпуск первым положением до заданного давления в УР
    if (pEQ >= p_charge + dp_over)
    {
        setBrakeCranePos(KRM_POS_II);
    }
    else
    {
        if (pEQ < p_charge - 0.01)
        {
            setBrakeCranePos(KRM_POS_I);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::brakeStepEPB(double pBC, double pBC_ref)
{
    if (pBC < pBC_ref + num_EPB_steps * 0.02)
    {
        if (!brakeEPB_timer->isStarted())
        {
            bc_state.brake_crane_pos_ref = KRM_POS_Va;
            brakeEPB_timer->start();
        }
    }
    else
    {
        setBrakeCranePos(KRM_POS_IV);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::brakeReleaseEPB(double pEQ, double pBC, double p_charge, double dp_over)
{
    if (pBC <= pBC_min)
    {
        return;
    }

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::slotBrakePBdelay()
{
    // Останавливаем таймер выдержки в ступени
    brakePB_timer->stop();

    // Оценка эффективности торможения по текущему ускорению
    // и ускорению на кривой снижения скорости
    if (a_cur > -a_ref)
    {
        // Добавляем дополнительную ступень разрядки
        num_PB_steps++;
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::slotBrakeEPBdelay()
{
    brakeEPB_timer->stop();

    if (a_cur > -a_ref)
    {
        // Добавляем дополнительную ступень
        num_EPB_steps++;
    }
}
