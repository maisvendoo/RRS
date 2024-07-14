#include    "kme-60-044.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKME_60_044::ControllerKME_60_044(QObject *parent)
    : TractionController(parent)
    , main_pos(POS_ZERO)
    , revers_pos(REVERS_ZERO)
    , main_handle_pos(0.0f)
    , revers_handle_pos(0.0f)
    , is_main_1_or_2(false)
    , is_revers_1_or_2(false)
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
km_state_t ControllerKME_60_044::getState() const
{
    return state;
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
void ControllerKME_60_044::setReversPos(int pos)
{
    revers_pos = pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ControllerKME_60_044::getSound(size_t idx)
{
    if (idx < sounds.size())
        return sounds[idx];
    return sound_state_t();
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

    state.revers_ref_state = cut(revers_pos - 1, -1, 1);

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
    if (getKeyState(KEY_A))
    {
        if (!incMainPos->isStarted())
            incMainPos->start();
    }
    else
    {
        incMainPos->stop();

        if (main_pos == POS_AP)
            decMain();
    }

    // Тянем рукоятку от себя
    if (getKeyState(KEY_D))
    {
        if (getKeyState(KEY_Control_L) || getKeyState(KEY_Control_R))
        {
            if (main_pos != POS_ZERO)
            {
                main_pos = POS_ZERO;
                soundMainChangePos();
            }
        }
        else
        {
            if (!decMainPos->isStarted())
                decMainPos->start();
        }
    }
    else
    {
        decMainPos->stop();

        if (main_pos == POS_BV)
            incMain();
    }

    // Тянем реверсивку от себя
    if (getKeyState(KEY_W))
    {
        if (!incReversPos->isStarted())
            incReversPos->start();
    }
    else
    {
        incReversPos->stop();
    }

    // Тянем реверсивку на себя
    if (getKeyState(KEY_S))
    {
        if (!decReversPos->isStarted())
            decReversPos->start();
    }
    else
    {
        decReversPos->stop();
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
    // Два звука по очереди, чтобы спокойно сбросить сигнал каждому
    if (is_main_1_or_2)
    {
        sounds[MAIN_CHANGE_POS_1].play = true;
        sounds[MAIN_CHANGE_POS_2].play = false;
        is_main_1_or_2 = false;
    }
    else
    {
        sounds[MAIN_CHANGE_POS_2].play = true;
        sounds[MAIN_CHANGE_POS_1].play = false;
        is_main_1_or_2 = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::soundReversChangePos()
{
    // Два звука по очереди, чтобы спокойно сбросить сигнал каждому
    if (is_revers_1_or_2)
    {
        sounds[MAIN_CHANGE_POS_1].play = true;
        sounds[MAIN_CHANGE_POS_2].play = false;
        is_revers_1_or_2 = false;
    }
    else
    {
        sounds[MAIN_CHANGE_POS_2].play = true;
        sounds[MAIN_CHANGE_POS_1].play = false;
        is_revers_1_or_2 = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::incMain()
{
    if (revers_pos == REVERS_ZERO)
        return;

    int main_pos_old = main_pos;

    main_pos++;

    main_pos = cut(main_pos, static_cast<int>(POS_BV), static_cast<int>(POS_AP));

    if (main_pos_old != main_pos)
        soundMainChangePos();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::decMain()
{
    if (revers_pos == REVERS_ZERO)
        return;

    int main_pos_old = main_pos;

    main_pos--;

    main_pos = cut(main_pos, static_cast<int>(POS_BV), static_cast<int>(POS_AP));

    if (main_pos_old != main_pos)
        soundMainChangePos();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::incRevers()
{
    if ( (revers_pos == REVERS_BACKWARD) && (main_pos != POS_ZERO) )
        return;

    int revers_pos_old = revers_pos;

    revers_pos++;

    revers_pos = cut(revers_pos, static_cast<int>(REVERS_BACKWARD), static_cast<int>(REVERS_OP3));

    if (revers_pos_old != revers_pos)
        soundReversChangePos();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKME_60_044::decRevers()
{
    if ( (revers_pos == REVERS_FORWARD) && (main_pos != POS_ZERO) )
        return;

    int revers_pos_old = revers_pos;

    revers_pos--;

    revers_pos = cut(revers_pos, static_cast<int>(REVERS_BACKWARD), static_cast<int>(REVERS_OP3));

    if (revers_pos_old != revers_pos)
        soundReversChangePos();
}
