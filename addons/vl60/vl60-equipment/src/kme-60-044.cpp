#include    "kme-60-044.h"

#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKME_60_044::ControllerKME_60_044(QObject *parent)
    : TractionController(parent)
    , main_pos(POS_ZERO)
    , revers_pos(REVERS_ZERO)
    , main_handle_pos(0.0f)
    , revers_handle_pos(0.0f)
{
    std::fill(sounds.begin(), sounds.end(), sound_state_t());

    incMainPos = new Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    connect(incMainPos, &Timer::process, this, &ControllerKME_60_044::incMain);

    decMainPos = new Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    connect(decMainPos, &Timer::process, this, &ControllerKME_60_044::decMain);

    incReversPos = new Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    connect(incReversPos, &Timer::process, this, &ControllerKME_60_044::incRevers);

    decReversPos = new Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    connect(decReversPos, &Timer::process, this, &ControllerKME_60_044::decRevers);

    positions_names << "BV" << " 0" << "AV" << "RV" << "FV" << "FP" << "RP" << "AP";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKME_60_044::~ControllerKME_60_044()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::insertReversHandle(bool insert)
{
    if (insert)
    {
        // Вставляем реверсивную рукоятку
        is_revers_handle.set();
        return;
    }

    // Извлечение реверсивной рукоятки только в нулевом положении
    if (revers_pos == REVERS_ZERO)
    {
        is_revers_handle.reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::setMainHandlePos(int pos)
{
    if ((pos < POS_BV) || (pos > POS_AP))
        return;

    // Блокировка поворота главной рукоятки при нулевом положении реверсивной
    if (revers_pos == REVERS_ZERO)
        pos = POS_ZERO;

    if (main_pos == pos)
        return;

    main_pos = pos;
    sounds[MAIN_CHANGE_POS_SOUND].play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::setReversHandlePos(int pos)
{
    // При извлечённой реверсивной рукоятке реверсивный вал всегда в нуле
    if (!is_revers_handle.getState())
    {
        revers_pos = REVERS_ZERO;
        return;
    }

    if ((pos < REVERS_BACKWARD) || (pos > REVERS_OP3))
        return;

    // При ненулевом положении главной рукоятки реверс не переключается через 0
    if (main_pos != POS_ZERO)
    {
        if (revers_pos == REVERS_BACKWARD)
            return;

        if ((revers_pos >= REVERS_FORWARD) && (pos < REVERS_FORWARD))
            return;
    }

    if (revers_pos == pos)
        return;

    revers_pos = pos;
    sounds[REVERS_CHANGE_POS_SOUND].play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ControllerKME_60_044::isReversHandle() const
{
    return is_revers_handle.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
km_state_t ControllerKME_60_044::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ControllerKME_60_044::getMainHandlePosName() const
{
    return positions_names[main_pos];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControllerKME_60_044::getMainHandlePos() const
{
    return main_handle_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControllerKME_60_044::getReversHandlePos() const
{
    return revers_handle_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ControllerKME_60_044::getSoundState(size_t idx) const
{
    if (idx < NUM_SOUNDS)
    {
        return sounds[idx];
    }

    return is_revers_handle.getSoundState(idx - NUM_SOUNDS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControllerKME_60_044::getSoundSignal(size_t idx) const
{
    if (idx < NUM_SOUNDS)
    {
        return sounds[idx].createSoundSignal();
    }

    return is_revers_handle.getSoundSignal(idx - NUM_SOUNDS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    std::fill(state.pos_state.begin(), state.pos_state.end(), false);
    state.pos_state[static_cast<size_t>(main_pos)] = true;

    state.revers_ref_state = std::clamp(revers_pos - 1, -1, 1);

    state.field_loosen_pos = pf(revers_pos - 2);

    if (main_pos < POS_ZERO)
        main_handle_pos = static_cast<float>(main_pos - 2) / 2.0f;
    else
        main_handle_pos = static_cast<float>(main_pos - 2) / 5.0f;

    if (revers_pos < 0)
        revers_handle_pos = static_cast<float>(revers_pos - 2);
    else
        revers_handle_pos = static_cast<float>(revers_pos - 1) / 4.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::ode_system(const state_vector_t &Y,
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
void ControllerKME_60_044::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::stepKeysControl(double t, double dt)
{
    // Тянем рукоятку на себя нажатием
    if (getKeyState(pressed_keys, KEY_A))
    {
        if (!incMainPos->isStarted())
            incMainPos->start();
    }
    else
    {
        incMainPos->stop();

        // Автовозврат
        if (main_pos == POS_AP)
            decMain();
    }

    // Тянем рукоятку от себя
    if (getKeyState(pressed_keys, KEY_D))
    {
        // Ctrl+D возвращает в нулевую позицию
        if (isModifier(pressed_keys, MODIFIER_OnlyControl))
        {
            setMainHandlePos(POS_ZERO);
            is_prev_KEY_D = true;
        }
        else
        {
            if ((!is_prev_KEY_D) && (!decMainPos->isStarted()))
                decMainPos->start();
        }
    }
    else
    {
        decMainPos->stop();
        is_prev_KEY_D = false;

        // Автовозврат
        if (main_pos == POS_BV)
            incMain();
    }

    // Тянем реверсивку от себя
    if (getKeyState(pressed_keys, KEY_W))
    {
        if (isModifier(pressed_keys, MODIFIER_OnlyShift))
        {
            // Shift - вставляем реверсивку
            insertReversHandle(true);
            is_prev_KEY_W = true;
        }
        else
        {
            if (isModifier(pressed_keys, MODIFIER_OnlyControl))
            {
                // Ctrl - извлекаем реверсивку
                insertReversHandle(false);
                is_prev_KEY_W = true;
            }
            else
            {
                if (is_revers_handle.getState())
                {
                    if (!incReversPos->isStarted() && (!is_prev_KEY_W))
                        incReversPos->start();
                }
                else
                {
                    incReversPos->stop();
                }
            }
        }
    }
    else
    {
        incReversPos->stop();
        is_prev_KEY_W = false;
    }

    // Тянем реверсивку на себя
    if (getKeyState(pressed_keys, KEY_S) && is_revers_handle.getState())
    {
        // Ctrl - быстрый возврат в нулевую позицию
        if (isModifier(pressed_keys, MODIFIER_OnlyControl))
        {
            setReversHandlePos(REVERS_ZERO);
            is_prev_KEY_S = true;
        }
        else
        {
            if ((!decReversPos->isStarted()) && (!is_prev_KEY_S))
                decReversPos->start();
        }
    }
    else
    {
        decReversPos->stop();
        is_prev_KEY_S = false;
    }

    incMainPos->step(t, dt);
    decMainPos->step(t, dt);
    incReversPos->step(t, dt);
    decReversPos->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::soundMainChangePos()
{
    sounds[MAIN_CHANGE_POS_SOUND].play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::soundReversChangePos()
{
    sounds[REVERS_CHANGE_POS_SOUND].play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::incMain()
{
    setMainHandlePos(main_pos + 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::decMain()
{
    setMainHandlePos(main_pos - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::incRevers()
{
    setReversHandlePos(revers_pos + 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::decRevers()
{
    setReversHandlePos(revers_pos - 1);
}
