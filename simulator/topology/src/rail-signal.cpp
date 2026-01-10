#include    "rail-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal::Signal(QObject *parent) : Device(parent)
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;

    std::fill(alsn_state.begin(), alsn_state.end(), false);

    alsn_RY_relay->read_config("combine-relay");
    alsn_RY_relay->setInitContactState(ALSN_RY, false);

    alsn_Y_relay->read_config("combine-relay");
    alsn_Y_relay->setInitContactState(ALSN_Y, false);

    alsn_G_relay->read_config("combine-relay");
    alsn_G_relay->setInitContactState(ALSN_G, false);

    connect(alsn_allow_timer, &Timer::process, this, &Signal::slotAllowTransmit);
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

    alsn_RY_relay->step(t, dt);
    alsn_Y_relay->step(t, dt);
    alsn_G_relay->step(t, dt);

    // Если АЛСН не запрещена, с задержкой по 10-секундному таймеру включаем трансмиттер
    if (is_alsn_allow && !is_asln_transmit)
    {
        if (!alsn_allow_timer->isStarted())
            alsn_allow_timer->start();

        alsn_allow_timer->step(t, dt);
    }

    // Сбрасываем запрет АЛСН
    is_alsn_allow = true;
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
    stream << signal_dir;
    stream << letter;
    stream << signal_model;

    for (auto lens : lens_state)
    {
        stream << lens;
    }

    this->calcPosition(pos);

    stream << pos.x << pos.y << pos.z;
    stream << orth.x << orth.y << orth.z;
    stream << right.x << right.y << right.z;
    stream << up.x << up.y << up.z;

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
    stream >> signal_dir;
    stream >> letter;
    stream >> signal_model;

    for (size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }

    stream >> pos.x >> pos.y >> pos.z;
    stream >> orth.x >> orth.y >> orth.z;
    stream >> right.x >> right.y >> right.z;
    stream >> up.x >> up.y >> up.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Signal::calcPosition(dvec3 &pos)
{
    dvec3 conn_pos;
    track_t track;

    if (!getConnectorPos(conn, conn_pos, track))
    {
        return false;
    }

    pos = conn_pos +
          track.trav * (rel_pos.x * signal_dir) +
          track.orth * (rel_pos.y * signal_dir) +
          track.up * rel_pos.z;

    // Делаем систему координат светофора вертикальной и повёрнутой в нужную сторону
    right = normalize(dvec3(track.trav.x * signal_dir, track.trav.y * signal_dir, 0.0));
    orth = normalize(dvec3(track.orth.x * signal_dir, track.orth.y * signal_dir, 0.0));
    up = dvec3(0.0, 0.0, 1.0);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::allowTransmitALSN(bool is_allow)
{
    is_alsn_allow = is_allow;
    is_asln_transmit = is_allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::load_config(CfgReader &cfg)
{
    (void)cfg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Signal::getConnectorPos(Connector *conn, dvec3 &conn_pos, track_t &track)
{
    if (conn == nullptr)
    {
        return false;
    }

    Trajectory *traj = nullptr;
    // dvec3 pos;

    if (signal_dir == 1)
    {
        traj = conn->getBwdTraj();

        if (traj == nullptr)
        {
            return false;
        }

        track = traj->getLastTrack();
        conn_pos = track.end_point;

        return true;
    }

    if (signal_dir == -1)
    {
        traj = conn->getFwdTraj();

        if (traj == nullptr)
        {
            return false;
        }

        track = traj->getFirstTrack();
        conn_pos = track.begin_point;

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::alsn_reset()
{
    std::fill(alsn_state.begin(), alsn_state.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::slotAllowTransmit()
{
    alsn_allow_timer->stop();
    is_asln_transmit = true;
}
