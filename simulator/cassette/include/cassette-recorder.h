//------------------------------------------------------------------------------
//
//      Cassette recorder system (кассета регистрации .kr)
//      ТЗ "Система кассет регистрации": бортовой "чёрный ящик"
//
//      Пишет файл <Корень>/Cassettes/<Дата>_<Локомотив>.kr:
//      бинарный формат с шифрованием AES-256-CBC, блочными CRC32,
//      мастер-хешем SHA-256 и дельта-сжатием аналоговых каналов.
//      Запись -append-чанками с периодическим сбросом на диск: при
//      краше данные до последнего сброса сохраняются (ТЗ п.9).
//
//      Все значения поступают из реальных физических систем через
//      CassetteFrame (модель их заполняет, ТЗ п.10).
//
//------------------------------------------------------------------------------

#ifndef     CASSETTE_RECORDER_H
#define     CASSETTE_RECORDER_H

#include    <QString>

#include    <cstdint>
#include    <fstream>
#include    <string>
#include    <vector>

//------------------------------------------------------------------------------
/// Кадр данных кассеты (заполняет модель из реальных систем)
//------------------------------------------------------------------------------
struct CassetteFrame
{
    // Аналоговые каналы (ТЗ п.2)
    double actual_speed = 0.0;          ///< км/ч
    double allowed_speed = 0.0;         ///< км/ч
    double brake_pipe_pressure = 0.0;   ///< кгс/см2 (ТМ)
    double equalizing_reservoir_pressure = 0.0; ///< кгс/см2 (УР, ТЗ п.2)
    double brake_cylinder_pressure = 0.0;   ///< кгс/см2 (ТЦ)
    double main_reservoir_pressure = 0.0;   ///< кгс/см2 (НМ/ГР)
    double traction_current = 0.0;      ///< А (ток ТЭД)
    double overhead_voltage = 0.0;      ///< В (КС)
    double traction_force = 0.0;        ///< тс
    double brake_force = 0.0;           ///< тс
    double controller_position = 0.0;   ///< позиция КМ
    double acceleration = 0.0;          ///< м/с2

    /// Направление движения: +1 вперёд / -1 назад (скатывание)
    double direction = 1.0;
    /// Уровень бодрствования ТСКБМ, % (0-100)
    double vigilance_level = 100.0;
    /// Режим ЭПТ: 0 выкл / 1 служебное / 2 экстренное
    double ept_mode = 0.0;

    // Дискретные каналы (ТЗ п.3, битовая маска)
    bool epk = false;               ///< ЭПК включён
    bool rb_pressed = false;        ///< рукоятка бдительности
    bool saut = false;              ///< САУТ
    bool tskbm = false;             ///< ТСКБМ
    bool compressor = false;        ///< компрессор
    bool fan = false;               ///< вентилятор
    bool sand = false;              ///< песок
    bool traction_mode = false;     ///< тяга
    bool regen_mode = false;        ///< рекуперация
    bool emergency_brake = false;   ///< экстренное торможение
    bool horn = false;              ///< свисток
    bool doors_open = false;        ///< двери
    bool lights = false;            ///< освещение
    bool heater = false;            ///< печь
    bool roof_raised = false;       ///< крышевое оборудование
    bool breaker_on = false;        ///< БВ включён

    // Профиль пути (ТЗ п.7)
    double coordinate = 0.0;        ///< пикетаж, м
    double gradient = 0.0;          ///< уклон, промилле
    double curvature = 0.0;         ///< кривизна, 1/м
    double path_speed_limit = 0.0;  ///< км/ч
};

//------------------------------------------------------------------------------
/// Кассета регистрации
//------------------------------------------------------------------------------
class CassetteRecorderSystem
{
public:

    enum class State
    {
        Idle,       ///< не пишем
        Recording,  ///< идёт запись
        Completed,  ///< файл закрыт корректно
        Error       ///< ошибка ввода-вывода
    };

    /// Событие лога нажатий (ТЗ п.4)
    struct ButtonEvent
    {
        double t = 0.0;             ///< с начала записи, с
        double coordinate = 0.0;    ///< м
        double speed = 0.0;         ///< км/ч
        std::uint16_t action_id = 0;
        std::string action_name;
        std::string parameter;
        std::uint8_t previous_state = 0;
        std::uint8_t new_state = 0;
        double duration = 0.0;      ///< удержание, с
    };

    CassetteRecorderSystem() = default;

    /// Загрузка секции [Cassette]
    void loadConfig(QString cfg_path);

    /// Система включена конфигом
    bool isEnabled() const;

    /// Начать запись: создаёт файл, серийный номер, заголовок
    /// и блок метаданных. locomotive - серия/номер из конфига ПС
    bool start(const QString& locomotive, double train_mass_t,
               double train_length_axles);

    /// Корректно завершить запись (расчёт хешей, футер)
    void stop();

    /// Добавить кадр (частота задаёт модель по SampleRate конфига)
    void sample(double t, const CassetteFrame& frame);

    /// Лог нажатия кнопки/переключателя
    void logButton(const ButtonEvent& event);

    /// Шаг: периодический сброс на диск (FlushInterval)
    void step(double dt);

    State getState() const;

    /// Имя текущего/последнего файла (для диагностики)
    QString getFileName() const;

    QString getDebugMsg() const;

private:

    /// Открытый файл (append-журнал блоков)
    std::fstream file;

    QString file_name;

    State state = State::Idle;

    bool enabled = true;
    double flush_interval = 5.0;
    double flush_timer = 0.0;

    /// Серийный номер кассеты (UUID-подобный из счётчика и времени)
    std::string serial;

    /// Накопители аналоговых каналов (с момента последнего сброса)
    struct AnalogSeries
    {
        std::uint16_t id = 0;
        double value = 0.0;     ///< предыдущее значение (для дельт)
        bool has_prev = false;
        std::vector<std::int64_t> deltas;   ///< квантованные дельты
        double t0 = 0.0;                    ///< время первого сэмпла
    };

    std::vector<AnalogSeries> analog;

    /// События дискретных каналов (по изменению маски)
    struct DiscreteEvent
    {
        double t = 0.0;
        std::uint32_t mask = 0;
    };

    std::vector<DiscreteEvent> discrete_events;
    std::uint32_t discrete_mask_prev = 0;
    bool discrete_has_prev = false;

    std::vector<ButtonEvent> buttons;

    /// Записи профиля пути (не чаще 1 Гц)
    struct PathRecord
    {
        double t = 0.0;
        double coordinate = 0.0;
        double gradient = 0.0;
        double curvature = 0.0;
        double limit = 0.0;
    };

    std::vector<PathRecord> path_records;
    double path_last_t = -1.0;

    /// Хеш-цепочка целостности: SHA-256 поверх записанных байтов
    std::vector<std::uint8_t> master_hash_state;

    std::uint64_t total_written = 0;
    std::uint64_t samples_total = 0;
    std::uint64_t buttons_total = 0;

    /// Записать блок: [block_id u32][длина payload u32][payload][crc u32]
    void writeBlock(std::uint32_t block_id,
                    const std::vector<std::uint8_t>& payload);

    /// Открытый блок (без шифрования): bootstrap с серийником -
    /// ключ сессии выводится из серийника, он нужен до расшифровки
    void writePlainBlock(std::uint32_t block_id,
                         const std::vector<std::uint8_t>& payload);

    /// Сбросить накопленные данные на диск
    void flush();

    /// Кадр -> аналоговые накопители + дискретные события + путь
    void accumulate(double t, const CassetteFrame& frame);
};

#endif // CASSETTE_RECORDER_H
