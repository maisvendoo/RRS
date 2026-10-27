//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

#include    "session-save-manager.h"

#include    <QDateTime>
#include    <QDir>
#include    <QDomDocument>
#include    <QDomElement>
#include    <QFile>
#include    <QSaveFile>
#include    <QXmlStreamWriter>

//------------------------------------------------------------------------------
/// CRC32 (IEEE 802.3), без внешних зависимостей
//------------------------------------------------------------------------------
static std::uint32_t crc32_ieee(const char *data, std::size_t size)
{
    static std::uint32_t table[256];
    static bool table_ready = false;

    if (!table_ready)
    {
        for (std::uint32_t i = 0; i < 256; ++i)
        {
            std::uint32_t c = i;

            for (int k = 0; k < 8; ++k)
            {
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }

            table[i] = c;
        }

        table_ready = true;
    }

    std::uint32_t crc = 0xFFFFFFFFu;

    for (std::size_t i = 0; i < size; ++i)
    {
        crc = table[(crc ^ static_cast<std::uint8_t>(data[i])) & 0xFFu] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFFu;
}

//------------------------------------------------------------------------------
// Количество файлов в ротации
//------------------------------------------------------------------------------
static const int SAVE_COUNT = 10;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SessionSaveManager::SessionSaveManager(QObject *parent)
    : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SessionSaveManager::~SessionSaveManager()
{
    stop_flag.store(true);
    queue_cv.notify_all();

    if (worker.joinable())
    {
        worker.join();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString SessionSaveManager::savePath(int index)
{
    return QString("session-save-%1.xml").arg(index, 2, 10, QChar('0'));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SessionSaveManager::init(const QString &cfg_path, const QString &base_dir)
{
    this->base_dir = base_dir;

    QFile cfg_file(cfg_path);

    if (cfg_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDomDocument doc;
        QString error = "";
        int line = 0;
        int column = 0;

        if (doc.setContent(&cfg_file, &error, &line, &column))
        {
            QDomElement root = doc.documentElement();

            if (!root.isNull())
            {
                enabled = (root.attribute("Enabled", "1") != "0");
                autoload = (root.attribute("AutoLoad", "1") != "0");
                save_interval = root.attribute("Interval", "600").toDouble();

                if (save_interval < 60.0)
                {
                    save_interval = 60.0;
                }
            }
        }

        cfg_file.close();
    }

    QDir().mkpath(base_dir + QDir::separator() + "saves");

    worker = std::thread([this]()
    {
        while (true)
        {
            session::session_state_t state;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [this]()
                {
                    return stop_flag.load() || has_pending;
                });

                if (stop_flag.load() && !has_pending)
                {
                    break;
                }

                state = std::move(pending_state);
                has_pending = false;
            }

            QString error = "";
            writeSave(state, &error);
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SessionSaveManager::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SessionSaveManager::saveInterval() const
{
    return save_interval;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SessionSaveManager::isAutoloadEnabled() const
{
    return autoload;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList SessionSaveManager::saveFiles() const
{
    QStringList files;
    QDir dir(base_dir + QDir::separator() + "saves");

    for (int i = 0; i < SAVE_COUNT; ++i)
    {
        const QString path = dir.filePath(savePath(i));

        if (QFile::exists(path))
        {
            files.append(path);
        }
    }

    return files;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString SessionSaveManager::latestValidSave(QString *error) const
{
    const QStringList files = saveFiles();

    for (const QString &path : files)
    {
        session::session_state_t state;
        QString load_error = "";

        if (loadSession(path, state, &load_error))
        {
            return path;
        }
    }

    if (error != nullptr)
    {
        *error = "No valid session save found";
    }

    return "";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SessionSaveManager::loadSession(const QString &path,
                                     session::session_state_t &state,
                                     QString *error) const
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (error != nullptr)
        {
            *error = QString("Can't open session save: %1").arg(path);
        }

        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    // Последняя строка - CRC32 тела файла
    const int last_nl = data.lastIndexOf('\n');

    if (last_nl < 0)
    {
        if (error != nullptr)
        {
            *error = "Session save has no CRC line";
        }

        return false;
    }

    QByteArray body = data.left(last_nl);
    QByteArray crc_line = data.mid(last_nl + 1).trimmed();

    const std::uint32_t crc = crc32_ieee(body.constData(),
                                         static_cast<std::size_t>(body.size()));

    if (crc_line.toUInt() != crc)
    {
        if (error != nullptr)
        {
            *error = QString("Session save CRC mismatch: %1").arg(path);
        }

        return false;
    }

    return deserialize(body, state, error);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SessionSaveManager::saveAsync(const session::session_state_t &state)
{
    if (!enabled)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        pending_state = state;
        has_pending = true;
    }

    queue_cv.notify_all();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray SessionSaveManager::serialize(const session::session_state_t &state) const
{
    QByteArray output;
    QXmlStreamWriter writer(&output);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");
    writer.writeStartElement("SessionSave");

    writer.writeAttribute("DateData", QString::number(state.date_data));
    writer.writeAttribute("TimeData", QString::number(state.time_data));
    writer.writeAttribute("SimulationSeconds", QString::number(state.simulation_seconds, 'f', 3));
    writer.writeAttribute("TimeString", state.time_string);
    writer.writeAttribute("Route", state.route_name);

    for (const auto &train : state.trains)
    {
        writer.writeStartElement("Train");

        writer.writeAttribute("Index", QString::number(train.train_idx));
        writer.writeAttribute("Name", train.name);
        writer.writeAttribute("TabNumber", QString::number(train.tab_number));
        writer.writeAttribute("Config", train.train_config);
        writer.writeAttribute("Trajectory", train.trajectory_name);
        writer.writeAttribute("Coord", QString::number(train.init_coord, 'f', 3));
        writer.writeAttribute("Direction", QString::number(train.direction));
        writer.writeAttribute("Velocity", QString::number(train.init_velocity, 'f', 3));

        for (const auto &vehicle : train.vehicles)
        {
            writer.writeStartElement("Vehicle");

            writer.writeAttribute("ModelIndex", QString::number(vehicle.model_index));
            writer.writeAttribute("RailwayCoord", QString::number(vehicle.railway_coord, 'f', 3));
            writer.writeAttribute("Velocity", QString::number(vehicle.velocity_kmh, 'f', 3));
            writer.writeAttribute("Config", vehicle.config_name);
            writer.writeAttribute("ErPressure", QString::number(vehicle.er_pressure, 'f', 3));
            writer.writeAttribute("PantographUp", vehicle.pantograph_up ? "1" : "0");
            writer.writeAttribute("Trajectory", vehicle.traj_name);
            writer.writeAttribute("TrajCoord", QString::number(vehicle.traj_coord, 'f', 3));
            writer.writeAttribute("TrajDir", QString::number(vehicle.traj_dir));

            writer.writeEndElement();
        }

        writer.writeEndElement();
    }

    for (const auto &sw : state.switches)
    {
        writer.writeStartElement("Switch");

        writer.writeAttribute("Name", sw.name);
        writer.writeAttribute("StateFwd", QString::number(sw.state_fwd));
        writer.writeAttribute("StateBwd", QString::number(sw.state_bwd));
        writer.writeAttribute("RefStateFwd", QString::number(sw.ref_state_fwd));
        writer.writeAttribute("RefStateBwd", QString::number(sw.ref_state_bwd));

        writer.writeEndElement();
    }

    for (const auto &client : state.clients)
    {
        writer.writeStartElement("Client");

        writer.writeAttribute("Id", QString::number(client.client_id));
        writer.writeAttribute("TabNumber", QString::number(client.tab_number));
        writer.writeAttribute("VehicleIdx", QString::number(client.vehicle_idx));
        writer.writeAttribute("CabIdx", QString::number(client.cab_idx));
        writer.writeAttribute("Connected", client.connected ? "1" : "0");

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return output;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SessionSaveManager::deserialize(const QByteArray &data,
                                     session::session_state_t &state,
                                     QString *error) const
{
    QDomDocument doc;
    QString parse_error = "";
    int line = 0;
    int column = 0;

    if (!doc.setContent(data, &parse_error, &line, &column))
    {
        if (error != nullptr)
        {
            *error = QString("Session save XML parse error at %1:%2: %3")
                    .arg(line).arg(column).arg(parse_error);
        }

        return false;
    }

    const QDomElement root = doc.documentElement();

    if (root.isNull() || (root.tagName() != "SessionSave"))
    {
        if (error != nullptr)
        {
            *error = "Session save: wrong root element";
        }

        return false;
    }

    state = session::session_state_t();

    state.date_data = root.attribute("DateData", "0").toUInt();
    state.time_data = root.attribute("TimeData", "0").toUInt();
    state.simulation_seconds = root.attribute("SimulationSeconds", "0").toDouble();
    state.time_string = root.attribute("TimeString", "");
    state.route_name = root.attribute("Route", "");

    QDomElement train_el = root.firstChildElement("Train");

    while (!train_el.isNull())
    {
        session::train_state_t train;

        train.train_idx = train_el.attribute("Index", "-1").toInt();
        train.name = train_el.attribute("Name", "");
        train.tab_number = train_el.attribute("TabNumber", "-1").toInt();
        train.train_config = train_el.attribute("Config", "");
        train.trajectory_name = train_el.attribute("Trajectory", "");
        train.init_coord = train_el.attribute("Coord", "0").toDouble();
        train.direction = train_el.attribute("Direction", "0").toInt();
        train.init_velocity = train_el.attribute("Velocity", "0").toDouble();

        QDomElement vehicle_el = train_el.firstChildElement("Vehicle");

        while (!vehicle_el.isNull())
        {
            session::vehicle_state_t vehicle;

            vehicle.model_index = vehicle_el.attribute("ModelIndex", "-1").toInt();
            vehicle.railway_coord = vehicle_el.attribute("RailwayCoord", "0").toDouble();
            vehicle.velocity_kmh = vehicle_el.attribute("Velocity", "0").toDouble();
            vehicle.config_name = vehicle_el.attribute("Config", "");
            vehicle.er_pressure = vehicle_el.attribute("ErPressure", "0").toDouble();
            vehicle.pantograph_up = (vehicle_el.attribute("PantographUp", "0") == "1");
            vehicle.traj_name = vehicle_el.attribute("Trajectory", "");
            vehicle.traj_coord = vehicle_el.attribute("TrajCoord", "0").toDouble();
            vehicle.traj_dir = vehicle_el.attribute("TrajDir", "0").toInt();

            train.vehicles.push_back(vehicle);

            vehicle_el = vehicle_el.nextSiblingElement("Vehicle");
        }

        state.trains.push_back(train);

        train_el = train_el.nextSiblingElement("Train");
    }

    QDomElement switch_el = root.firstChildElement("Switch");

    while (!switch_el.isNull())
    {
        session::switch_state_t sw;

        sw.name = switch_el.attribute("Name", "");
        sw.state_fwd = switch_el.attribute("StateFwd", "0").toInt();
        sw.state_bwd = switch_el.attribute("StateBwd", "0").toInt();
        sw.ref_state_fwd = switch_el.attribute("RefStateFwd", "0").toInt();
        sw.ref_state_bwd = switch_el.attribute("RefStateBwd", "0").toInt();

        state.switches.push_back(sw);

        switch_el = switch_el.nextSiblingElement("Switch");
    }

    QDomElement client_el = root.firstChildElement("Client");

    while (!client_el.isNull())
    {
        session::client_state_t client;

        client.client_id = client_el.attribute("Id", "-1").toInt();
        client.tab_number = client_el.attribute("TabNumber", "-1").toInt();
        client.vehicle_idx = client_el.attribute("VehicleIdx", "-1").toInt();
        client.cab_idx = client_el.attribute("CabIdx", "-1").toInt();
        client.connected = (client_el.attribute("Connected", "0") == "1");

        state.clients.push_back(client);

        client_el = client_el.nextSiblingElement("Client");
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SessionSaveManager::writeSave(const session::session_state_t &state,
                                   QString *error)
{
    rotate();

    const QDir dir(base_dir + QDir::separator() + "saves");
    const QString path = dir.filePath(savePath(0));

    const QByteArray body = serialize(state);

    const std::uint32_t crc = crc32_ieee(body.constData(),
                                         static_cast<std::size_t>(body.size()));

    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (error != nullptr)
        {
            *error = QString("Can't open session save for writing: %1").arg(path);
        }

        return false;
    }

    file.write(body);
    file.write("\n");
    file.write(QByteArray::number(crc));
    file.write("\n");

    if (!file.commit())
    {
        if (error != nullptr)
        {
            *error = QString("Can't commit session save: %1").arg(path);
        }

        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Сдвиг кольца: 08 -> 09, ... 00 -> 01
//------------------------------------------------------------------------------
void SessionSaveManager::rotate() const
{
    const QDir dir(base_dir + QDir::separator() + "saves");

    for (int i = SAVE_COUNT - 2; i >= 0; --i)
    {
        const QString src = dir.filePath(savePath(i));
        const QString dst = dir.filePath(savePath(i + 1));

        if (QFile::exists(src))
        {
            QFile::remove(dst);
            QFile::rename(src, dst);
        }
    }
}
