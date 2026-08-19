#include    "trac-transformer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TracTransformer::TracTransformer(QObject *parent) : Device(parent)
    , U1(0.0)
    , K_sn(62.7)
    , ekg_pos(0)
    , sound_state(sound_state_t(true, 0.0f))
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TracTransformer::~TracTransformer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracTransformer::setPosition(int position)
{
    ekg_pos = position;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracTransformer::setU1(double value)
{
    U1 = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TracTransformer::getU_sn() const
{
    return U1 / K_sn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TracTransformer::getTracVoltage() const
{
    return cur_pos.Unom * U1 / 25000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString TracTransformer::getPosName() const
{
    return cur_pos.name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t TracTransformer::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float TracTransformer::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state.createSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracTransformer::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (position.contains(ekg_pos))
    {
        cur_pos = position[ekg_pos];
    }

    sound_state.volume = U1 / 25000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TracTransformer::ode_system(const state_vector_t &Y,
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
void TracTransformer::load_config(CfgReader &cfg)
{
    QDomNode secNode = cfg.getFirstSection("Position");

    while (!secNode.isNull())
    {
        position_t pos;

        cfg.getInt(secNode, "Number", pos.number);
        cfg.getDouble(secNode, "Voltage", pos.Unom);
        cfg.getString(secNode, "Name", pos.name);

        position.insert(pos.number, pos);

        secNode = cfg.getNextSection();
    }
}
