#include    "km-35-01.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TracController::TracController(QObject *parent) : Device(parent)
{
    tracTimer.setTimeout(0.1);
    connect(&tracTimer, &Timer::process,
            this, &TracController::slotTracLevelProcess);

    brakeTimer.setTimeout(0.1);
    connect(&brakeTimer, &Timer::process,
            this, &TracController::slotBrakeLevelProcess);

    speedTimer.setTimeout(0.1);
    connect(&speedTimer, &Timer::process,
            this, &TracController::slotSpeedLevelProcess);
    speedTimer.start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TracController::~TracController()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::allowReversHandle(bool allow)
{
    is_reverse_handle_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TracController::isReversHandleAllowed() const
{
    return is_reverse_handle_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::insertReversHandle(bool insert)
{
    insert = insert && is_reverse_handle_allowed;

    if (insert)
    {
        // Вставляем реверсивную рукоятку
        is_revers_handle.set();
        return;
    }

    // Извлечение реверсивной рукоятки только в нулевом положении
    if (revers_pos == 0)
    {
        is_revers_handle.reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TracController::isReversHandle() const
{
    return is_revers_handle.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float TracController::getHandlePosition() const
{
    float handle_pos = mode_pos * 0.2f + trac_level / 125.0f - brake_level / 125.0f;

    return handle_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float TracController::getSoundSignal(size_t state_idx) const
{
    if (state_idx < NUM_SOUNDS)
    {
        return sound_states[state_idx].createSoundSignal();
    }

    return is_revers_handle.getSoundSignal(state_idx - NUM_SOUNDS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (mode_pos != mode_pos_old)
    {
        sound_states[MAIN_CHANGE_POS_SOUND].play(true);
        mode_pos_old = mode_pos;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::ode_system(const state_vector_t &Y,
                                state_vector_t &dYdt,
                                double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double timeout = 0.1;
    cfg.getDouble(secName, "handle_motion_time", timeout);
    if (timeout > Physics::ZERO)
    {
        brakeTimer.setTimeout(timeout);
        tracTimer.setTimeout(timeout);
    }

    int coeff = 1;
    cfg.getInt(secName, "handle_high_speed_coeff", coeff);
    if ((coeff > 1) && (coeff < 100))
    {
        handle_high_speed_coeff = coeff;
    }

    timeout = 0.1;
    cfg.getDouble(secName, "refV_motion_time", timeout);
    if (timeout > Physics::ZERO)
    {
        speedTimer.setTimeout(timeout);
    }
    speedTimer.start();

    coeff = 1;
    cfg.getInt(secName, "refV_high_speed_coeff", coeff);
    if ((coeff > 1) && (coeff < 100))
    {
        refV_high_speed_coeff = static_cast<double>(coeff);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::stepKeysControl(double t, double dt)
{
    bool key_fwd = getKeyState(pressed_keys, KEY_W);
    bool key_bwd = getKeyState(pressed_keys, KEY_S);
    bool key_traction = getKeyState(pressed_keys, KEY_A);
    bool key_v_ref_inc = getKeyState(pressed_keys, KEY_Q);
    bool key_brakes = getKeyState(pressed_keys, KEY_D);
    bool key_v_ref_dec = getKeyState(pressed_keys, KEY_E);
    bool isShift = isModifier(pressed_keys, MODIFIER_OnlyShift);
    bool isControl = isModifier(pressed_keys, MODIFIER_OnlyControl);

    // Управление реверсивной рукояткой
    if (key_fwd)
    {
        if (isShift)
        {
            // Shift - вставляем реверсивку
            insertReversHandle(true);
        }
        else
        {
            if (isControl)
            {
                // Ctrl - извлекаем реверсивку
                insertReversHandle(false);
            }
            else
            {
                if (isReversHandle())
                {
                    if (!old_fwd_key_state && isZero() && (revers_pos < 1))
                    {
                        // Тянем реверсивку от себя
                        revers_pos++;

                        sound_states[REVERS_CHANGE_POS_SOUND].play();
                    }
                }
                else
                {
                    revers_pos = 0;
                }
            }
        }
    }

    if (key_bwd)
    {
        if (isControl)
        {
            if (!old_bwd_key_state && isZero() && (revers_pos != 0))
            {
                // Ctrl - быстрый возврат в нулевую позицию
                revers_pos = 0;

                sound_states[REVERS_CHANGE_POS_SOUND].play();
            }
        }
        else
        {
            if (isReversHandle())
            {
                if (!old_bwd_key_state && isZero() && (revers_pos > -1))
                {
                    // Тянем реверсивку на себя
                    revers_pos--;

                    sound_states[REVERS_CHANGE_POS_SOUND].play();
                }
            }
            else
            {
                revers_pos = 0;
            }
        }
    }

    old_fwd_key_state = key_fwd;
    old_bwd_key_state = key_bwd;

    // Управление контроллером
    if ((revers_pos != 0) && (mode_pos == 0))
    {
        trac_level = brake_level = 0;
        traction.reset();
        brake.reset();
        processDiscretePositions(key_traction, old_traction_key, 1);
        processDiscretePositions(key_brakes, old_brake_key, -1);
    }

    // Тут реализуем процесс перемещения главной рукоятки!!!

    if (mode_pos == -1)
    {
        traction.reset();
        handle_motion_speed = 0;

        if (!brakeTimer.isStarted())
            brakeTimer.start();

        if (key_traction)
        {
            if (brake_level == 0)
            {
                mode_pos = 0;
                brakeTimer.stop();
                brake.reset();
            }
            else
            {
                if (isShift)
                    handle_motion_speed = handle_high_speed_coeff;
                else
                    handle_motion_speed = 1;
            }
        }

        if (key_brakes)
        {
            if (brake.getState())
            {
                if (isShift)
                    handle_motion_speed = -handle_high_speed_coeff;
                else
                    handle_motion_speed = -1;
            }

            if (isControl)
            {
                mode_pos = 0;
                brakeTimer.stop();
                brake.reset();
            }
        }
        else
        {
            brake.set();
        }
    }

    brakeTimer.step(t, dt);

    if (mode_pos == 1)
    {
        brake.reset();
        handle_motion_speed = 0;

        if (!tracTimer.isStarted())
            tracTimer.start();

        if (key_brakes)
        {
            if ( (trac_level == 0) || isControl )
            {
                mode_pos = 0;
                tracTimer.stop();
                traction.reset();
            }
            else
            {
                if (isShift)
                    handle_motion_speed = -handle_high_speed_coeff;
                else
                    handle_motion_speed = -1;
            }
        }

        if (key_traction)
        {
            if (traction.getState())
            {
                if (isShift)
                    handle_motion_speed = handle_high_speed_coeff;
                else
                    handle_motion_speed = 1;
            }
        }
        else
        {
            traction.set();
        }
    }

    tracTimer.step(t, dt);

    old_traction_key = key_traction;
    old_brake_key = key_brakes;

    refV_motion_speed = 0.0;

    if (key_v_ref_inc)
    {
        if (isShift)
            refV_motion_speed = refV_high_speed_coeff;
        else
            refV_motion_speed = 1.0;
    }

    if (key_v_ref_dec)
    {
        if (isControl)
        {
            refV_motion_speed = 0.0;
            refV_level = 0.0;
        }
        else
        {
            if (isShift)
                refV_motion_speed = -refV_high_speed_coeff;
            else
                refV_motion_speed = -1.0;
        }
    }

    speedTimer.step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::processDiscretePositions(bool key_state,
                                              bool old_key_state,
                                              int dir)
{
    if (key_state && !old_key_state)
    {
        mode_pos += dir;
        mode_pos = cut(mode_pos, std::int8_t(-1), std::int8_t(1));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::slotTracLevelProcess()
{
    trac_level += handle_motion_speed * mode_pos;

    trac_level = cut(trac_level, 0, 100);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::slotBrakeLevelProcess()
{
    brake_level += handle_motion_speed * mode_pos;

    brake_level = cut(brake_level, 0, 100);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracController::slotSpeedLevelProcess()
{
    refV_level += refV_motion_speed * refV_step;

    refV_level = cut(refV_level, 0.0, 1.0);
}
