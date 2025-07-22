#include    "ekg-8g.h"

#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EKG_8G::EKG_8G(QObject *parent) : Device(parent)
    , position(0)
    , ref_position(0)
    , switch_time(1.0)
    , is_enabled(false)
    , is_ready(false)
    , is_LK_allow(false)
    , is_fix_start(false)
    , is_fix_off(false)
    , dir(0)
    , is_auto(false)
    , is_sound_one_or_auto(NO_SOUND)
{
    connect(&pos_switcher, &Timer::process, this, &EKG_8G::slotPosSwitch);

    std::fill(is_long_motion.begin(), is_long_motion.end(), false);
    std::fill(sounds.begin(), sounds.end(), sound_state_t());

    is_long_motion[LM_POS0] = true;
    is_long_motion[LM_POS1] = true;
    is_long_motion[LM_POS2] = true;
    is_long_motion[LM_POS3] = true;
    is_long_motion[LM_POS4] = true;
    is_long_motion[LM_POS5] = true;
    is_long_motion[LM_POS6] = true;
    is_long_motion[LM_POS7] = true;
    is_long_motion[LM_POS8] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EKG_8G::~EKG_8G()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::setKMstate(const km_state_t &km1_state, const km_state_t &km2_state)
{
    this->km1_state = km1_state;
    this->km2_state = km2_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float EKG_8G::getSelsinPosition() const
{
    return static_cast<float>(position) / (NUM_POSITIONS - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EKG_8G::isLongMotionPos() const
{
    return is_long_motion[static_cast<size_t>(position)];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::enable(bool value)
{
    is_enabled = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int EKG_8G::getPosition() const
{
    return position;
}

bool EKG_8G::isReady() const
{
    return is_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EKG_8G::isLKallow() const
{
    return is_LK_allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::process()
{
    // Нулевая позиция
    bool is_KM_ZERO = km1_state.pos_state[POS_ZERO] && km2_state.pos_state[POS_ZERO];

    if (is_KM_ZERO && (position != 0) && !is_auto)
    {
        is_sound_one_or_auto = SOUND_AUTO;
        pos_switcher.start();
        dir = -1;
    }

    // Фиксация пуска
    if (km1_state.pos_state[POS_FP] || km2_state.pos_state[POS_FP])
    {
        is_sound_one_or_auto = NO_SOUND;
        is_fix_start = true;
        dir = 1;
    }

    // Ручной пуск
    if ( (km1_state.pos_state[POS_RP] || km2_state.pos_state[POS_RP])  && is_fix_start)
    {
        is_sound_one_or_auto = SOUND_ONE;
        is_fix_start = false;
        pos_switcher.start();
    }

    // Фиксация выключения
    if (km1_state.pos_state[POS_FV] || km2_state.pos_state[POS_FV])
    {
        is_sound_one_or_auto = NO_SOUND;
        is_fix_off = true;
        dir = -1;
    }

    // Ручное выключение
    if ((km1_state.pos_state[POS_RV] || km2_state.pos_state[POS_RV]) && is_fix_off)
    {
        is_sound_one_or_auto = SOUND_ONE;
        is_fix_off = false;
        pos_switcher.start();
    }

    // Автоматический пуск
    if ((km1_state.pos_state[POS_AP] || km2_state.pos_state[POS_AP]) && !is_auto)
    {
        is_sound_one_or_auto = SOUND_AUTO;
        pos_switcher.start();
        dir = 1;
    }

    // Автоматическое выключение
    if ((km1_state.pos_state[POS_AV] || km2_state.pos_state[POS_AV]) && !is_auto)
    {
        is_sound_one_or_auto = SOUND_AUTO;
        pos_switcher.start();
        dir = -1;
    }

    is_auto = km1_state.pos_state[POS_AV] || km1_state.pos_state[POS_AP] ||
        km2_state.pos_state[POS_AV] || km2_state.pos_state[POS_AP] || is_KM_ZERO;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t EKG_8G::getSoundState(size_t idx) const
{
    if (idx < sounds.size())
        return sounds[idx];
    return Device::getSoundState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float EKG_8G::getSoundSignal(size_t idx) const
{
    if (idx < sounds.size())
        return sounds[idx].createSoundSignal();
    return Device::getSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Если на ЭКГ подано питание
    if (is_enabled)
    {
        if (is_ready)
            process();
        else
        {
            // Устанавливаем готовность по состоянию рукоятки КМ
            is_ready = km1_state.pos_state[POS_ZERO] && km2_state.pos_state[POS_ZERO];
        }
    }
    else
    {
        pos_switcher.stop();
        is_ready = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::ode_system(const state_vector_t &Y,
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
void EKG_8G::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    int switch_interval = 0;

    if (cfg.getInt(secName, "PosSwitchTime", switch_interval))
    {
        switch_time = static_cast<double>(switch_interval) / NUM_POSITIONS;
    }

    pos_switcher.setTimeout(switch_time);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::stepDiscrete(double t, double dt)
{
    pos_switcher.step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::slotPosSwitch()
{
    // Набирам/сбрасываем позицию
    position += dir;

    position = std::clamp(position, 0, static_cast<int>(NUM_POSITIONS - 1));

    // Озвучка
    if ( (position != 0) && (position != NUM_POSITIONS - 1) )
    {
        // Звук ручного переключения
        if (is_sound_one_or_auto == SOUND_ONE)
        {
            sounds[CHANGE_POS_ONE_SOUND].play();
        }
        // Звук автоматического переключения
        if (is_sound_one_or_auto == SOUND_AUTO)
        {
            sounds[CHANGE_POS_AUTO_SOUND].play();
        }
    }

        // Останавливаемся, если не находимся на переходных позициях
    if ( ( (position < PP_MIN) || (position > PP_MAX) ) && !is_auto )
        pos_switcher.stop();
}
