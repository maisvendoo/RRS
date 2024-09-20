#include    "rail-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal::Signal(QObject *parent) : Device(parent)
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal::~Signal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::step(double t, double dt)
{
    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Signal::serialize()
{
    QByteArray tmp_data;
    QBuffer buff(&tmp_data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << conn->getName();
    stream << is_busy;
    stream << letter;
    stream << signal_dir;

    for (auto lens : lens_state)
    {
        stream << lens;
    }

    return buff.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> conn_name;
    stream >> is_busy;
    stream >> letter;
    stream >> signal_dir;

    for (size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::load_config(CfgReader &cfg)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::slotRecvLineVoltage(double U_line)
{
    this->U_line = U_line;
}
