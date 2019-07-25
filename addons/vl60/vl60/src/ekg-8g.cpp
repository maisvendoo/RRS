#include    "ekg-8g.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EKG_8G::EKG_8G(QObject *parent) : Device(parent)
  , position(0)
  , ref_position(0)
  , switch_time(1.0)
{
    connect(&pos_switcher, &Timer::process, this, &EKG_8G::slotPosSwitch);
    pos_switcher.start();
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
void EKG_8G::setKMstate(const km_state_t &km_state)
{
    this->km_state = km_state;
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
void EKG_8G::preStep(state_vector_t &Y, double t)
{
    if (km_state.pos_state[POS_AV])
    {
        if (ref_position == position)
            ref_position--;
    }

    if (km_state.pos_state[POS_FV])
        fix_off.set();

    if ( km_state.pos_state[POS_RV] && fix_off.getState() )
    {
        if ( (position > PP_MIN) && (position <= PP_MAX) )
            ref_position = PP_MIN;
        else
            ref_position--;

        fix_off.reset();
    }

    if (km_state.pos_state[POS_FP])
        fix_start.set();

    if ( km_state.pos_state[POS_RP] && fix_start.getState() )
    {
        if ( (position >= PP_MIN) && (position < PP_MAX) )
            ref_position = PP_MAX;
        else
            ref_position++;

        fix_start.reset();
    }

    if (km_state.pos_state[POS_AP])
    {
        if (ref_position == position)
            ref_position++;
    }

    ref_position = cut(ref_position, 0, static_cast<int>(NUM_POSITIONS - 1));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::ode_system(const state_vector_t &Y,
                        state_vector_t &dYdt,
                        double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EKG_8G::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    int switch_interval = 0;

    if (cfg.getInt(secName, "PosSwitchIntelval", switch_interval))
    {
        switch_time = static_cast<double>(switch_interval) / 1000.0;
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
    if (ref_position != position)
        emit soundPlay("EKG_serv");

    position += sign(ref_position - position);
}
