#include    "rail-signal.h"

#include    "filesystem.h"
#include    "CfgReader.h"
#include    "Journal.h"
#include    "switch.h"
#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal::Signal(QObject* parent) : QObject(parent)
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal::~Signal() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::step(double t, double dt)
{
    (void)dt;

    preStep(t);

    if (old_lens_state != lens_state)
    {
        old_lens_state = lens_state;
        emit sendDataUpdate(serialize());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::read_config(const QString &filename, const QString &dir_path)
{
    FileSystem &fs = FileSystem::getInstance();
    CfgReader cfg;

    // Custom config from path
    if (dir_path != "")
    {
        QString cfg_path = dir_path + QDir::separator() + filename + ".xml";

        if (cfg.load(cfg_path))
        {
            Journal::instance()->info("Loaded file: " + cfg_path);

            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + dir_path);
        }
    }

    // Config from default directory
    QString cfg_dir = fs.getDevicesDir().c_str();
    QString cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded file: " + cfg_path);

        load_config(cfg);
        return;
    }

    Journal::instance()->error("File " + filename + ".xml is't found at default path " + cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Signal::getConnectorName() const
{
    return conn_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setConnector(Switch* conn)
{
    this->conn = conn;
    conn_name = conn ? conn->getName() : "";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch* Signal::getConnector() const
{
    return conn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setDirection(dir_t dir)
{
    signal_dir = dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dir_t Signal::getDirection() const
{
    return signal_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setLetter(const QString& letter)
{
    this->letter = letter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Signal::getLetter() const
{
    return letter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setSignalModel(const QString& signal_model)
{
    this->signal_model = signal_model;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Signal::getSignalModel() const
{
    return this->signal_model;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Signal::getSignalType() const
{
    return signal_model.right(4);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
lens_state_t Signal::getAllLensState() const
{
    return lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setRelPosition(const dvec3& rel_pos)
{
    this->rel_pos = rel_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::setRelRotation(const dvec3& rel_rot)
{
    this->rel_rot = rel_rot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Signal::serialize()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << conn_name;
    stream << signal_dir;
    stream << letter;
    stream << signal_model;

    for (bool lens : lens_state)
    {
        stream << lens;
    }

    calcPosition();

    stream << pos.x << pos.y << pos.z;
    stream << orth.x << orth.y << orth.z;
    stream << right.x << right.y << right.z;
    stream << up.x << up.y << up.z;

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> conn_name;
    stream >> signal_dir;
    stream >> letter;
    stream >> signal_model;

    for (bool& lens : lens_state)
    {
        stream >> lens;
    }

    stream >> pos.x >> pos.y >> pos.z;
    stream >> orth.x >> orth.y >> orth.z;
    stream >> right.x >> right.y >> right.z;
    stream >> up.x >> up.y >> up.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Signal::calcPosition()
{
    // Смотрим на коннектор при светофоре
    if (conn == nullptr)
    {
        return false;
    }

    // Смотрим на траекторию перед коннектором при светофоре
    const Trajectory* traj = nullptr;
    if (signal_dir == 1)
    {
        traj = conn->trajectories[SW_BWD_PLUS]
            ? conn->trajectories[SW_BWD_PLUS]
            : conn->trajectories[SW_BWD_MINUS];
    }
    else if (signal_dir == -1)
    {
        traj = conn->trajectories[SW_FWD_PLUS]
            ? conn->trajectories[SW_FWD_PLUS]
            : conn->trajectories[SW_FWD_MINUS];
    }

    if (traj == nullptr)
    {
        return false;
    }

    // Берём за точку отсчёта участок траектории перед коннектором
    const double d = signal_dir * conn->getTrajOrientation(traj);
    const track_t& track = (d > 0.0) ? traj->getLastTrack() : traj->getFirstTrack();
    const dvec3 conn_pos = (d > 0.0) ? track.end_point : track.begin_point;

    // Положение светофора со смещением от коннектора
    pos = conn_pos +
          track.trav * (rel_pos.x * d) +
          track.orth * (rel_pos.y * d) +
          track.up * rel_pos.z;

    // Делаем систему координат светофора вертикальной и повёрнутой в нужную сторону
    right = normalize(dvec3(track.trav.x * d, track.trav.y * d, 0.0));
    orth = normalize(dvec3(track.orth.x * d, track.orth.y * d, 0.0));
    up = dvec3(0.0, 0.0, 1.0);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getPos() const
{
    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getRight() const
{
    return right;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getOrth() const
{
    return orth;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getUp() const
{
    return up;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getRelPos() const
{
    return rel_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const dvec3& Signal::getRelRot() const
{
    return rel_rot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::preStep(double t)
{
    (void)t;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signal::load_config(CfgReader &cfg)
{
    (void)cfg;
}
