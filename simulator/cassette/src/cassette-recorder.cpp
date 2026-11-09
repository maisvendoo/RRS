//------------------------------------------------------------------------------
//
//      Cassette recorder system (кассета регистрации .kr)
//
//------------------------------------------------------------------------------

#include    "cassette-recorder.h"
#include    "kr-crypto.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <QDateTime>
#include    <QDir>
#include    <QFileInfo>
#include    <QString>

#include    <algorithm>
#include    <cmath>
#include    <cstring>

namespace
{

/// Мастер-ключ формата. Встраивается в симулятор и в программу
/// расшифровки; в файле кассеты НЕ хранится (ТЗ п.8: без ключа
/// прочитать данные невозможно). Изменение ломает все старые кассеты
const std::uint8_t KR_MASTER_KEY[32] =
{
    0x8e, 0x12, 0x7a, 0x45, 0xd3, 0x6b, 0x9f, 0x21,
    0xc8, 0x54, 0x0e, 0x77, 0x3b, 0xaa, 0x91, 0x2d,
    0x64, 0xf0, 0x18, 0xc5, 0x9e, 0x37, 0x72, 0x0b,
    0xad, 0x49, 0x86, 0xe1, 0x53, 0x0c, 0xbf, 0x25
};

/// Сигнатура формата (16 байт)
const char KR_MAGIC[16] =
{
    'R', 'R', 'S', '-', 'K', 'R', '-', 'C', 'A', 'S', 'S', 'E', 'T', 'T', 'E', '\0'
};

/// Версия формата
constexpr std::uint16_t KR_FORMAT_VERSION = 1;

/// Идентификаторы блоков
constexpr std::uint32_t BLOCK_BOOTSTRAP = 0;     ///< открытый серийник
constexpr std::uint32_t BLOCK_SESSION_INFO = 1;
constexpr std::uint32_t BLOCK_ANALOG_CHUNK = 5;
constexpr std::uint32_t BLOCK_DISCRETE_CHUNK = 6;
constexpr std::uint32_t BLOCK_BUTTON_CHUNK = 7;
constexpr std::uint32_t BLOCK_PATH_CHUNK = 8;
constexpr std::uint32_t BLOCK_INTEGRITY = 9;

/// Маркер конца файла (8 байт)
const char KR_END_MARKER[8] = { 'K', 'R', '-', 'E', 'N', 'D', '\0', '\0' };

//------------------------------------------------------------------------------
/// Младшие байты числа little-endian
//------------------------------------------------------------------------------
template <typename T>
void putLE(std::vector<std::uint8_t>& out, const T& value)
{
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&value);

    static_assert(sizeof(T) <= 8);

    for (std::size_t i = 0; i < sizeof(T); ++i)
    {
        out.push_back(bytes[i]);
    }
}

//------------------------------------------------------------------------------
/// ZigZag-varint (LEB128): компактная запись знаковых дельт
//------------------------------------------------------------------------------
void putVarint(std::vector<std::uint8_t>& out, std::int64_t value)
{
    const std::uint64_t zz = (static_cast<std::uint64_t>(value) << 1) ^
            (value < 0 ? ~0ull : 0ull);

    std::uint64_t rest = zz;

    while (rest >= 0x80u)
    {
        out.push_back(static_cast<std::uint8_t>(rest) | 0x80u);
        rest >>= 7;
    }

    out.push_back(static_cast<std::uint8_t>(rest));
}

void putString(std::vector<std::uint8_t>& out, const std::string& value)
{
    putLE<std::uint16_t>(out, static_cast<std::uint16_t>(value.size()));

    for (char c : value)
    {
        out.push_back(static_cast<std::uint8_t>(c));
    }
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Cassette";

    cfg.getBool(sec, "Enabled", enabled);
    cfg.getDouble(sec, "FlushInterval", flush_interval);

    flush_interval = std::min(std::max(flush_interval, 1.0), 60.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CassetteRecorderSystem::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CassetteRecorderSystem::start(const QString& locomotive,
                                   double train_mass_t,
                                   double train_length_axles)
{
    if (state == State::Recording)
        return true;

    // Каталог и имя файла: Cassettes/<Дата>_<Локомотив>.kr (ТЗ п.1)
    const QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    const QString dir = "Cassettes";

    QDir().mkpath(dir);

    QString base = QString("%1_%2").arg(date).arg(locomotive);

    // Не затираем существующую кассету той же даты: добавляем индекс
    QString name = base + ".kr";

    for (int index = 1; QFileInfo::exists(QDir(dir).filePath(name)); ++index)
    {
        name = QString("%1_%2.kr").arg(base).arg(index);
    }

    file_name = QDir(dir).filePath(name);

    file.open(file_name.toStdString().c_str(),
              std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        state = State::Error;
        Journal::instance()->error("[CASSETTE] Cannot create file: " + file_name);
        return false;
    }

    // Серийный номер кассеты: метка времени + счётчик инстансов
    // (уникален в пределах запуска, ТЗ п.6)
    static std::uint32_t instance_counter = 0;
    ++instance_counter;

    serial = QString("%1-%2")
            .arg(QDateTime::currentMSecsSinceEpoch(), 0, 16)
            .arg(instance_counter, 4, 16, QChar('0'))
            .toStdString();

    //--- Заголовок файла: magic + версия + флаг шифрования + CRC ---
    std::vector<std::uint8_t> header;

    for (std::uint8_t byte : KR_MAGIC)
    {
        header.push_back(byte);
    }

    putLE<std::uint16_t>(header, KR_FORMAT_VERSION);
    header.push_back(1);    // CiphertextFlag: блоки шифруются

    const std::uint32_t header_crc = kr::crc32(header);

    putLE<std::uint32_t>(header, header_crc);

    file.write(reinterpret_cast<const char*>(header.data()), header.size());
    total_written += header.size();

    //--- Открытый серийник кассеты (bootstrap): ключ сессии выводится
    // из серийника, поэтому он обязан быть читаем без расшифровки.
    // Подделка серийника обнаружится подписью и мастер-хешем ---
    std::vector<std::uint8_t> bootstrap;
    putString(bootstrap, serial);
    writePlainBlock(BLOCK_BOOTSTRAP, bootstrap);

    //--- Блок метаданных (ТЗ п.6) ---
    std::vector<std::uint8_t> meta;

    putString(meta, serial);
    putString(meta, locomotive.toStdString());
    putString(meta, QDateTime::currentDateTime()
                 .toString("yyyy-MM-dd hh:mm:ss").toStdString());

    putLE<double>(meta, train_mass_t);
    putLE<double>(meta, train_length_axles);

    // Список аналоговых каналов (ТЗ п.2): ID, имя, ед., диапазон, Гц
    const std::vector<std::uint16_t> analog_ids =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    const char* analog_names[] =
    {
        "ActualSpeed", "AllowedSpeed", "BrakePipePressure",
        "BrakeCylinderPressure", "MainReservoirPressure",
        "TractionMotorCurrent", "OverheadLineVoltage", "TractionForce",
        "BrakeForce", "ControllerPosition", "Acceleration",
        "EqualizingReservoirPressure", "Direction", "VigilanceLevel",
        "EPTMode"
    };

    const char* analog_units[] =
    {
        "km/h", "km/h", "kgf/cm2", "kgf/cm2", "kgf/cm2",
        "A", "V", "tf", "tf", "pos", "m/s2", "kgf/cm2", "dir", "%",
        "mode"
    };

    const double analog_max[] =
    {
        400.0, 400.0, 10.0, 6.0, 10.0,
        2000.0, 40000.0, 100.0, 100.0, 60.0, 5.0, 6.0, 1.0, 100.0, 2.0
    };

    putLE<std::uint16_t>(meta, static_cast<std::uint16_t>(analog_ids.size()));

    for (std::size_t i = 0; i < analog_ids.size(); ++i)
    {
        putLE<std::uint16_t>(meta, analog_ids[i]);
        putString(meta, analog_names[i]);
        putString(meta, analog_units[i]);
        putLE<float>(meta, 0.0f);
        putLE<float>(meta, static_cast<float>(analog_max[i]));
        putLE<float>(meta, 2.0f);   ///< SampleRate, Гц (скорость/ТМ)
    }

    // Дискретные каналы (ТЗ п.3): 16 битов маски, имена
    const char* discrete_names[] =
    {
        "EPK", "RB", "SAUT", "TSKBM", "Compressor", "Fan", "SandSystem",
        "TractionMode", "RegenMode", "EmergencyBrake", "Horn", "Doors",
        "LightsOn", "HeaterOn", "RoofEquipment", "BV",
        "ControlGenerator", "EPKKey", "Whistle"};

    putLE<std::uint16_t>(meta, 19);

    for (const char* name : discrete_names)
    {
        putString(meta, name);
    }

    writeBlock(BLOCK_SESSION_INFO, meta);

    // Накопители каналов
    analog.clear();
    analog.resize(analog_ids.size());

    for (std::size_t i = 0; i < analog_ids.size(); ++i)
    {
        analog[i].id = analog_ids[i];
    }

    discrete_events.clear();
    discrete_has_prev = false;
    buttons.clear();
    path_records.clear();
    path_last_t = -1.0;

    samples_total = 0;
    buttons_total = 0;
    flush_timer = 0.0;

    state = State::Recording;

    Journal::instance()->info("[CASSETTE] Recording started: " + file_name +
                              " serial " + QString::fromStdString(serial));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::stop()
{
    if (state != State::Recording)
        return;

    flush();

    //--- Блок целостности: мастер-хеш SHA-256 всех записанных байтов,
    // отпечаток ключа сессии и подпись формата (ТЗ п.8) ---
    std::vector<std::uint8_t> file_bytes;

    file.flush();
    file.close();
    file.open(file_name.toStdString().c_str(), std::ios::in | std::ios::binary);

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        file_bytes.resize(static_cast<std::size_t>(size));

        if (size > 0)
        {
            file.read(reinterpret_cast<char*>(file_bytes.data()), size);
        }

        file.close();
    }

    const std::vector<std::uint8_t> master_hash = kr::sha256(file_bytes);

    // Отпечаток ключа сессии: sha256(session_key)[0..8]
    std::vector<std::uint8_t> key_material;
    key_material.assign(KR_MASTER_KEY, KR_MASTER_KEY + 32);

    for (char c : serial)
    {
        key_material.push_back(static_cast<std::uint8_t>(c));
    }

    const std::vector<std::uint8_t> session_key = kr::sha256(key_material);

    const std::vector<std::uint8_t> key_fingerprint =
            kr::sha256(session_key);

    // Подпись: sha256(мастер-ключ || мастер-хеш || метка формата)
    std::vector<std::uint8_t> signature_input;
    signature_input.assign(KR_MASTER_KEY, KR_MASTER_KEY + 32);
    signature_input.insert(signature_input.end(),
                           master_hash.begin(), master_hash.end());

    for (char c : std::string("RRS-KR-SIG"))
    {
        signature_input.push_back(static_cast<std::uint8_t>(c));
    }

    const std::vector<std::uint8_t> signature =
            kr::sha256(signature_input);

    std::vector<std::uint8_t> integrity;

    for (std::uint8_t byte : master_hash)
    {
        integrity.push_back(byte);
    }

    for (std::size_t i = 0; i < 8; ++i)
    {
        integrity.push_back(key_fingerprint[i]);
    }

    for (std::uint8_t byte : signature)
    {
        integrity.push_back(byte);
    }

    file.open(file_name.toStdString().c_str(),
              std::ios::out | std::ios::binary | std::ios::app);

    if (!file.is_open())
    {
        state = State::Error;
        return;
    }

    writeBlock(BLOCK_INTEGRITY, integrity);

    //--- Футер: маркер конца + CRC32 всего файла ---
    std::vector<std::uint8_t> footer;

    for (std::uint8_t byte : KR_END_MARKER)
    {
        footer.push_back(byte);
    }

    // CRC пишем в конец после его вычисления: сначала маркер
    file.write(reinterpret_cast<const char*>(footer.data()), footer.size());
    total_written += footer.size();

    // Перечитываем файл (без последних 4 байт CRC) для контрольной суммы
    file.flush();
    file.close();
    file.open(file_name.toStdString().c_str(), std::ios::in | std::ios::binary);

    std::uint32_t total_crc = 0;

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<std::uint8_t> all(static_cast<std::size_t>(size));
        file.read(reinterpret_cast<char*>(all.data()), size);
        file.close();

        total_crc = kr::crc32(all);
    }

    std::vector<std::uint8_t> crc_bytes;
    putLE<std::uint32_t>(crc_bytes, total_crc);

    file.open(file_name.toStdString().c_str(),
              std::ios::out | std::ios::binary | std::ios::app);
    file.write(reinterpret_cast<const char*>(crc_bytes.data()), 4);
    file.flush();
    file.close();

    state = State::Completed;

    Journal::instance()->info(
        QString("[CASSETTE] Recording completed: %1 (%2 samples, %3 buttons)")
            .arg(file_name)
            .arg(samples_total)
            .arg(buttons_total));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::sample(double t, const CassetteFrame& frame)
{
    if (state != State::Recording)
        return;

    accumulate(t, frame);
    ++samples_total;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::logButton(const ButtonEvent& event)
{
    if (state != State::Recording)
        return;

    buttons.push_back(event);
    ++buttons_total;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::step(double dt)
{
    if (state != State::Recording)
        return;

    flush_timer += dt;

    if (flush_timer >= flush_interval)
    {
        flush_timer = 0.0;
        flush();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CassetteRecorderSystem::State CassetteRecorderSystem::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CassetteRecorderSystem::getFileName() const
{
    return file_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CassetteRecorderSystem::getDebugMsg() const
{
    QString state_text = "IDLE";

    switch (state)
    {
    case State::Recording:
        state_text = "RECORDING";
        break;
    case State::Completed:
        state_text = "COMPLETED";
        break;
    case State::Error:
        state_text = "ERROR";
        break;
    default:
        break;
    }

    return QString("Cassette: %1, file %2, samples %3, buttons %4")
            .arg(state_text)
            .arg(file_name)
            .arg(samples_total)
            .arg(buttons_total);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::writePlainBlock(std::uint32_t block_id,
                                             const std::vector<std::uint8_t>& payload)
{
    // Открытый блок: без шифрования (bootstrap с серийником)
    std::vector<std::uint8_t> block;
    putLE<std::uint32_t>(block, block_id);
    putLE<std::uint32_t>(block, static_cast<std::uint32_t>(payload.size()));
    block.insert(block.end(), payload.begin(), payload.end());

    const std::uint32_t block_crc = kr::crc32(block);
    putLE<std::uint32_t>(block, block_crc);

    file.write(reinterpret_cast<const char*>(block.data()), block.size());
    total_written += block.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::writeBlock(std::uint32_t block_id,
                                        const std::vector<std::uint8_t>& payload)
{
    // Ключ сессии: sha256(мастер-ключ || серийник)
    std::vector<std::uint8_t> key_material;
    key_material.assign(KR_MASTER_KEY, KR_MASTER_KEY + 32);

    for (char c : serial)
    {
        key_material.push_back(static_cast<std::uint8_t>(c));
    }

    const std::vector<std::uint8_t> session_key = kr::sha256(key_material);

    // IV блока: sha256(серийник || счётчик блоков)
    std::vector<std::uint8_t> iv_material;

    for (char c : serial)
    {
        iv_material.push_back(static_cast<std::uint8_t>(c));
    }

    std::uint64_t block_seq = total_written;

    for (std::size_t i = 0; i < 8; ++i)
    {
        iv_material.push_back(static_cast<std::uint8_t>(block_seq >> (8u * i)));
    }

    const std::vector<std::uint8_t> iv_hash = kr::sha256(iv_material);

    std::vector<std::uint8_t> iv(iv_hash.begin(), iv_hash.begin() + 16);

    std::vector<std::uint8_t> cipher;

    if (!kr::aes256CbcEncrypt(session_key, iv, payload, cipher))
    {
        state = State::Error;
        return;
    }

    std::vector<std::uint8_t> block;
    putLE<std::uint32_t>(block, block_id);
    putLE<std::uint32_t>(block, static_cast<std::uint32_t>(cipher.size()));
    block.insert(block.end(), cipher.begin(), cipher.end());

    const std::uint32_t block_crc = kr::crc32(block);
    putLE<std::uint32_t>(block, block_crc);

    file.write(reinterpret_cast<const char*>(block.data()), block.size());
    total_written += block.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::accumulate(double t, const CassetteFrame& frame)
{
    const double values[] =
    {
        frame.actual_speed,
        frame.allowed_speed,
        frame.brake_pipe_pressure,
        frame.brake_cylinder_pressure,
        frame.main_reservoir_pressure,
        frame.traction_current,
        frame.overhead_voltage,
        frame.traction_force,
        frame.brake_force,
        frame.controller_position,
        frame.acceleration,
        frame.equalizing_reservoir_pressure,
        frame.direction,
        frame.vigilance_level,
        frame.ept_mode
    };

    // Дельта-кодирование аналоговых каналов (ТЗ п.8): квантование 0.01
    for (std::size_t i = 0; i < analog.size() && i < 15; ++i)
    {
        AnalogSeries& series = analog[i];

        if (!series.has_prev)
        {
            series.has_prev = true;
            series.t0 = t;
            series.value = values[i];
            series.deltas.push_back(
                        static_cast<std::int64_t>(std::llround(values[i] * 100.0)));
            continue;
        }

        const std::int64_t delta =
                static_cast<std::int64_t>(std::llround(
                    (values[i] - series.value) * 100.0));

        series.value = values[i];
        series.deltas.push_back(delta);
    }

    // Дискретные каналы: маска 16 битов, события по изменению (ТЗ п.3)
    const std::uint32_t mask =
            (frame.epk ? 0x0001u : 0u) |
            (frame.rb_pressed ? 0x0002u : 0u) |
            (frame.saut ? 0x0004u : 0u) |
            (frame.tskbm ? 0x0008u : 0u) |
            (frame.compressor ? 0x0010u : 0u) |
            (frame.fan ? 0x0020u : 0u) |
            (frame.sand ? 0x0040u : 0u) |
            (frame.traction_mode ? 0x0080u : 0u) |
            (frame.regen_mode ? 0x0100u : 0u) |
            (frame.emergency_brake ? 0x0200u : 0u) |
            (frame.horn ? 0x0400u : 0u) |
            (frame.doors_open ? 0x0800u : 0u) |
            (frame.lights ? 0x1000u : 0u) |
            (frame.heater ? 0x2000u : 0u) |
            (frame.roof_raised ? 0x4000u : 0u) |
            (frame.breaker_on ? 0x8000u : 0u);

    if (!discrete_has_prev)
    {
        discrete_has_prev = true;
        discrete_events.push_back({t, mask});
    }
    else if (mask != discrete_mask_prev)
    {
        discrete_events.push_back({t, mask});
    }

    discrete_mask_prev = mask;

    // Профиль пути: не чаще 1 Гц (ТЗ п.7)
    if (t - path_last_t >= 1.0)
    {
        path_last_t = t;
        path_records.push_back({t, frame.coordinate, frame.gradient,
                                frame.curvature, frame.path_speed_limit});
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CassetteRecorderSystem::flush()
{
    if (!file.is_open())
    {
        state = State::Error;
        return;
    }

    //--- Чанки аналоговых каналов: дельты в zigzag-varint (сжатие) ---
    for (const AnalogSeries& series : analog)
    {
        if (series.deltas.empty())
            continue;

        std::vector<std::uint8_t> payload;
        putLE<std::uint16_t>(payload, series.id);
        putLE<float>(payload, static_cast<float>(series.t0));
        putLE<std::uint32_t>(payload,
                             static_cast<std::uint32_t>(series.deltas.size()));

        for (std::int64_t delta : series.deltas)
        {
            putVarint(payload, delta);
        }

        writeBlock(BLOCK_ANALOG_CHUNK, payload);
    }

    //--- Дискретные события ---
    if (!discrete_events.empty())
    {
        std::vector<std::uint8_t> payload;
        putLE<std::uint32_t>(payload,
                             static_cast<std::uint32_t>(discrete_events.size()));

        for (const DiscreteEvent& event : discrete_events)
        {
            putLE<float>(payload, static_cast<float>(event.t));
            putLE<std::uint32_t>(payload, event.mask);
        }

        writeBlock(BLOCK_DISCRETE_CHUNK, payload);
    }

    //--- Лог нажатий ---
    if (!buttons.empty())
    {
        std::vector<std::uint8_t> payload;
        putLE<std::uint32_t>(payload,
                             static_cast<std::uint32_t>(buttons.size()));

        for (const ButtonEvent& event : buttons)
        {
            putLE<double>(payload, event.t);
            putLE<double>(payload, event.coordinate);
            putLE<float>(payload, static_cast<float>(event.speed));
            putLE<std::uint16_t>(payload, event.action_id);
            putString(payload, event.action_name);
            putString(payload, event.parameter);
            putLE<std::uint8_t>(payload, event.previous_state);
            putLE<std::uint8_t>(payload, event.new_state);
            putLE<float>(payload, static_cast<float>(event.duration));
        }

        writeBlock(BLOCK_BUTTON_CHUNK, payload);
    }

    //--- Профиль пути ---
    if (!path_records.empty())
    {
        std::vector<std::uint8_t> payload;
        putLE<std::uint32_t>(payload,
                             static_cast<std::uint32_t>(path_records.size()));

        for (const PathRecord& record : path_records)
        {
            putLE<double>(payload, record.t);
            putLE<double>(payload, record.coordinate);
            putLE<float>(payload, static_cast<float>(record.gradient));
            putLE<float>(payload, static_cast<float>(record.curvature));
            putLE<float>(payload, static_cast<float>(record.limit));
        }

        writeBlock(BLOCK_PATH_CHUNK, payload);
    }

    // Очистка накопителей
    for (auto& series : analog)
    {
        series.deltas.clear();
        series.has_prev = false;
    }

    discrete_events.clear();
    buttons.clear();
    path_records.clear();

    file.flush();
}
