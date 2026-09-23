//------------------------------------------------------------------------------
//
//      Conductors system (проводники пассажирского поезда)
//      ТЗ "Система проводников пассажирского поезда"
//
//      Лёгкий специализированный NPC (п.22 ТЗ): проводник существует
//      только во время обслуживания поезда на станции. Ядро симуляции
//      без 3D-моделей и анимаций (их время придёт позже, набор анимаций
//      п.21 ТЗ отражён фазами автомата):
//      - конечный автомат SERVICE_COUPÉ -> ... -> DESPAWN (п.2 ТЗ);
//      - двери OPEN/CLOSING/CLOSED/BLOCKED (п.17);
//      - лестница на низкой платформе (п.6);
//      - тип платформы ПО РАЗНОСТИ ВЫСОТ, не по имени (п.5);
//      - красный/жёлтый флаг готовности (п.10);
//      - внештатные ситуации, включая опаздывающего пассажира,
//        с детерминированным RNG на seed вагона (п.14-15);
//      - оптимизация NPC ACTIVE/INACTIVE/DESPAWN (п.19).
//
//      Позиции хранятся ОТНОСИТЕЛЬНО вагона (DoorSocket + смещение,
//      п.3 ТЗ): мировые координаты считает потребитель (вьювер) через
//      трансформ вагона. Система координат вагона: x - вдоль оси
//      (к голове поезда), y - вбок (положительное - сторона платформы),
//      z - вверх от головки рельса.
//
//      Потоки: контекстные сеттеры (setStationZone и т.п.) вызываются
//      моделью ДО выдачи шага поездам, step() - из Train::slotStep
//      в потоке поезда; Model::process не начинает новый тик, пока не
//      завершились шаги всех поездов, поэтому гонок нет.
//
//------------------------------------------------------------------------------

#ifndef     CONDUCTOR_SYSTEM_H
#define     CONDUCTOR_SYSTEM_H

#include    "conductor-export.h"

#include    <QString>

#include    <cstddef>
#include    <cstdint>
#include    <map>
#include    <vector>

namespace conductor
{

//------------------------------------------------------------------------------
/// Смещение в системе координат вагона (не мировые координаты!)
//------------------------------------------------------------------------------
struct LocalOffset
{
    double x = 0.0;     ///< Вдоль оси вагона, м (+ к голове поезда)
    double y = 0.0;     ///< Вбок, м (+ сторона платформы)
    double z = 0.0;     ///< Вверх от головки рельса, м
};

//------------------------------------------------------------------------------
/// Параметры системы из секции [Conductor] конфига поезда
//------------------------------------------------------------------------------
struct Config
{
    /// Система включена (при выключенной поезд всегда "готов")
    bool enabled = true;

    /// Порог высокой платформы: высота поверхности над головкой рельса,
    /// м (п.5 ТЗ; >= порога - HIGH, ниже - LOW)
    double platform_high_threshold = 0.9;

    /// Длительность укладки/уборки лестницы, с (п.6 ТЗ)
    double stairs_deploy_time = 6.0;

    /// Вероятность опаздывающего пассажира на остановку, 0..1 (п.14)
    double late_passenger_probability = 0.15;

    /// Задержка появления опаздывающего пассажира, с (п.14)
    double late_passenger_delay = 8.0;

    /// Ходьба от служебного купе к вагону, с (п.8 ТЗ)
    double walk_time = 12.0;

    /// Возврат в служебное купе, с
    double return_time = 10.0;

    /// Проверка вагона перед закрытием двери, с (п.9 шаг 12)
    double check_time = 5.0;

    /// Открытие двери, с
    double door_open_time = 2.5;

    /// Закрытие двери, с (п.17: состояние CLOSING)
    double door_close_time = 3.0;

    /// Вероятность блокировки двери пассажиром при закрытии, 0..1 (п.15)
    double door_blocked_probability = 0.05;

    /// Число попыток закрытия до объявления неисправности (п.17)
    int door_not_closed_retries = 3;

    /// Базовый seed детерминированного RNG (мешается с индексом вагона)
    std::uint64_t seed = 0;
};

//------------------------------------------------------------------------------
/// Данные платформы у вагона (задаются моделью по конфигу зоны станции)
//------------------------------------------------------------------------------
struct PlatformInfo
{
    /// Платформа обнаружена (поезд в зоне станции)
    bool known = false;

    /// Высота ПОВЕРХНОСТИ платформы над головкой рельса, м.
    /// Тип определяется разностью высот платформы и вагона,
    /// а не именем объекта (п.5 ТЗ)
    double height_above_rail = 0.0;

    /// Сторона вагона: +1 - платформа со стороны +y, -1 - с -y
    int side = 1;
};

//------------------------------------------------------------------------------
/// Описание вагона для привязки проводника (заполняет Train)
//------------------------------------------------------------------------------
struct WagonInfo
{
    /// Индекс ПЕ в модели (уникален между поездами)
    size_t vehicle_idx = 0;

    /// Признак пассажирского вагона (секция [PassengerCar])
    bool passenger = false;

    /// У вагона есть откидная/выдвижная лестница (п.6 ТЗ)
    bool has_stairs = true;

    /// Длина вагона по осям автосцепок, м
    double length = 25.0;

    /// Ширина кузова, м (для проверки габарита рабочей позиции)
    double width = 3.1;

    /// Розетка двери (DoorSocket) в системе координат вагона, п.3 ТЗ
    LocalOffset door_socket;

    /// Служебное купе (место спавна/возврата), п.3 ТЗ
    LocalOffset coupe_offset;
};

//------------------------------------------------------------------------------
/// Состояния конечного автомата проводника (п.2 ТЗ):
/// SERVICE_COUPÉ -> WAITING_FOR_TRAIN -> GOING_TO_WAGON -> WORKING ->
/// READY -> DEPARTURE -> RETURNING -> SERVICE_COUPÉ -> DESPAWN -> цикл
//------------------------------------------------------------------------------
enum class State
{
    DESPAWN = 0,        ///< NPC удалён из сцены (п.19), ждёт остановку
    SERVICE_COUPE,      ///< В служебном купе (появление/возврат)
    WAITING_FOR_TRAIN,  ///< Вышел из купе, ожидает начала обслуживания
    GOING_TO_WAGON,     ///< Идёт к назначенной двери вагона
    WORKING,            ///< Обслуживание: лестница/дверь/высадка/посадка
    READY,              ///< Жёлтый флаг: вагон готов к отправлению
    DEPARTURE,          ///< Подтверждение отправления получено
    RETURNING           ///< Возврат в служебное купе
};

//------------------------------------------------------------------------------
/// Фазы состояния WORKING (шаги основного цикла, п.9 ТЗ)
//------------------------------------------------------------------------------
enum class WorkPhase
{
    NONE = 0,
    DEPLOYING_STAIRS,   ///< Опускает лестницу (низкая платформа, п.6)
    OPENING_DOOR,       ///< Открывает дверь
    ALIGHTING,          ///< Контроль высадки (п.16)
    BOARDING,           ///< Контроль посадки
    CHECKING,           ///< Проверка готовности вагона
    CLOSING_DOOR,       ///< Закрывает дверь (п.17)
    RETRACTING_STAIRS,  ///< Убирает лестницу
    STUCK               ///< Дверь неисправна - красный флаг (п.17)
};

//------------------------------------------------------------------------------
/// Состояния двери (п.17 ТЗ)
//------------------------------------------------------------------------------
enum class DoorState
{
    OPEN = 0,
    CLOSING,
    CLOSED,
    BLOCKED
};

//------------------------------------------------------------------------------
/// Состояния лестницы
//------------------------------------------------------------------------------
enum class StairsState
{
    RETRACTED = 0,
    DEPLOYING,
    DEPLOYED,
    RETRACTING
};

//------------------------------------------------------------------------------
/// Тип платформы по разности высот (п.5 ТЗ)
//------------------------------------------------------------------------------
enum class PlatformType
{
    UNKNOWN = 0,
    LOW,                ///< Разность высот ниже порога
    HIGH                ///< Разность высот >= порога
};

//------------------------------------------------------------------------------
/// Сигнальный флаг проводника (п.10 ТЗ)
//------------------------------------------------------------------------------
enum class Flag
{
    NONE = 0,           ///< Проводник не у двери (идёт/в купе/DESPAWN)
    RED,                ///< NOT_READY: посадка/дверь/лестница/нештатная
    YELLOW              ///< READY: вагон готов к отправлению
};

//------------------------------------------------------------------------------
/// Внештатные ситуации (п.14-15 ТЗ)
//------------------------------------------------------------------------------
enum class Emergency
{
    NONE = 0,
    LATE_PASSENGER,         ///< Опаздывающий пассажир бежит к вагону
    DOOR_NOT_CLOSED,        ///< Дверь не закрывается - неисправность
    PASSENGER_STILL_BOARDING, ///< Пассажиры ещё садятся/выходят
    PASSENGER_BLOCKING_DOOR   ///< Пассажир блокирует дверь
};

//------------------------------------------------------------------------------
/// Активность NPC (п.19 ТЗ)
//------------------------------------------------------------------------------
enum class NpcActivity
{
    ACTIVE = 0,     ///< Полный шаг: модель/анимация/коллизии/позиция
    INACTIVE,       ///< Далеко от игрока: только логическое состояние
    DESPAWN         ///< Удалён из сцены
};

//------------------------------------------------------------------------------
/// Статистика NPC-пула (ТЗ "Оптимизация", п.13): для профайлера
//------------------------------------------------------------------------------
struct NpcStats
{
    std::size_t pool_size = 0;      ///< Объектов создано (размер пула)
    std::size_t bound = 0;          ///< Привязано к вагонам сейчас
    std::size_t active = 0;         ///< ACTIVE (полная симуляция)
    std::size_t inactive = 0;       ///< INACTIVE ("спящие": только логика)
    std::size_t despawned = 0;      ///< DESPAWN (вне сцены)
    std::size_t reused = 0;         ///< Переиспользовано между привязками
};

//------------------------------------------------------------------------------
//
//      Проводник пассажирского вагона
//
//------------------------------------------------------------------------------
class CONDUCTOR_EXPORT Conductor
{
public:

    /// wagon - описание вагона, cfg - параметры системы (система-
    /// владелец живёт дольше проводника), seed - seed детерминированного
    /// RNG вагона (п.14 ТЗ)
    Conductor(const WagonInfo& wagon, const Config* cfg, std::uint64_t seed);

    ~Conductor() = default;

    /// Метка для сообщений журнала (имя поезда, задаёт система)
    void setLabel(const QString& label);

    /// Перепривязка к другому вагону (объектный пул, ТЗ "Оптимизация"
    /// п.13): полный сброс автомата и новое состояние БЕЗ аллокации.
    /// Эквивалентен созданию нового проводника
    void rebind(const WagonInfo& wagon, std::uint64_t seed);

    /// Шаг конечного автомата. Контекст: поезд движется / поезд в зоне
    /// станции / игрок рядом (ACTIVE - полная симуляция, INACTIVE -
    /// только логика, п.19 ТЗ)
    void step(double dt, bool train_moving, bool in_station_zone,
              bool player_near);

    //--- Текущее состояние ---

    State getState() const;
    WorkPhase getWorkPhase() const;
    DoorState getDoorState() const;
    StairsState getStairsState() const;
    PlatformType getPlatformType() const;
    Flag getFlag() const;
    Emergency getEmergency() const;
    NpcActivity getActivity() const;

    /// Жёлтый флаг: вагон готов (посадка завершена, дверь закрыта,
    /// лестница убрана, проверка пройдена, нештатных нет), п.10-11 ТЗ
    bool isReady() const;

    //--- Позиции относительно вагона (мировую считает вьювер) ---

    /// Служебное купе (место спавна/возврата), п.3 ТЗ
    LocalOffset getServiceCoupeOffset() const;

    /// Розетка двери вагона (DoorSocket), п.3 ТЗ
    LocalOffset getDoorSocketOffset() const;

    /// Рабочая позиция у двери: DoorSocket + смещение наружу от борта
    /// на поверхность платформы (п.4, 18 ТЗ)
    LocalOffset getWorkingPositionOffset() const;

    /// Текущая позиция: интерполяция ходьбы купе<->дверь (только для
    /// ACTIVE; INACTIVE мгновенно занимает цель позиции, п.19 ТЗ)
    LocalOffset getCurrentPositionOffset() const;

    /// Направление взгляда, рад (в системе координат вагона)
    double getHeading() const;

    //--- Внешние данные (поток модели, до шага поезда) ---

    /// Данные платформы: высота над головкой рельса и сторона (п.5 ТЗ)
    void setPlatform(const PlatformInfo& info);

    /// Состояние пассажирских потоков вагона (п.16 ТЗ):
    /// true - идёт посадка / идёт высадка
    void setPassengerState(bool boarding, bool alighting);

    /// Посадка активна (разрешение станции, шаг системы поезда)
    void setBoardingActive(bool active);

    /// Команда машиниста REQUEST_DEPARTURE_READY: ускоренная финальная
    /// проверка (п.13 ТЗ)
    void setHurry(bool hurry);

    //--- Одноразовые запросы к интеграции с пассажирской системой ---

    /// Требование начать высадку (true один раз за фазу высадки):
    /// Model вызывает beginAlighting(текущее число пассажиров)
    bool fetchBeginAlightingRequest();

    /// Требование начать посадку (true один раз за фазу посадки).
    /// out_count: >0 - точное число садящихся (опаздывающий, п.14),
    /// 0 - по конфигу зоны (очередь LoadingPoint / вместимость/2)
    bool fetchBeginBoardingRequest(int& out_count);

    /// Подтверждение: поток пассажиров действительно запущен
    /// (число вышедших/садящихся > 0)
    void markAlightingDelivered(int count);
    void markBoardingDelivered(int count);

    //--- Служебное ---

    size_t getVehicleIdx() const;
    std::uint64_t getSeed() const;

private:

    /// Спавн перед началом посадки (п.3 ТЗ)
    void spawn();

    /// Определение типа платформы по разности высот (п.5 ТЗ)
    void determinePlatform();

    /// Начало обслуживания у вагона (п.9 шаги 5-7)
    void beginWorking();

    /// Шаг фаз состояния WORKING (п.9 ТЗ)
    void stepWorking(double dt);

    /// Начать закрытие двери (счётчик попыток, п.17 ТЗ)
    void enterClosingDoor();

    /// Шаг состояния READY: опаздывающий пассажир (п.14 ТЗ)
    void stepReady(double dt);

    /// Завершение работы: жёлтый флаг + бросок опаздывающего (п.10, 14)
    void finishWorking();

    /// Опаздывающий пассажир добежал: возврат к посадке (п.14 ТЗ)
    void triggerLatePassenger();

    /// Поезд тронулся: подтверждение отправления (п.9 шаг 16)
    void beginDeparture();

    /// Возврат в купе завершён: сброс, DESPAWN (п.2, 19 ТЗ)
    void resetService();

    /// Расчёт рабочей позиции с проверкой PositionValid (п.18 ТЗ)
    void calcWorkingPosition();

    /// Детерминированный RNG (SplitMix64, одинаков на всех платформах)
    std::uint64_t nextRand();
    double nextUniform();

    /// Префикс сообщений журнала
    QString logPrefix() const;

    WagonInfo wagon_;
    const Config* cfg_ = nullptr;   ///< Параметры системы-владельца
    std::uint64_t seed_ = 1;
    std::uint64_t rng_ = 1;
    QString label_ = "поезд";

    //--- Конечный автомат ---
    State state_ = State::DESPAWN;
    WorkPhase work_phase_ = WorkPhase::NONE;
    WorkPhase after_open_phase_ = WorkPhase::ALIGHTING;
    double state_timer_ = 0.0;
    double phase_timer_ = 0.0;
    NpcActivity activity_ = NpcActivity::DESPAWN;
    bool hurry_ = false;
    bool despawn_next_ = false;     ///< После купе уйти в DESPAWN

    //--- Дверь и лестница ---
    DoorState door_ = DoorState::CLOSED;
    StairsState stairs_ = StairsState::RETRACTED;
    bool stairs_used_ = false;
    int close_attempts_ = 0;

    //--- Платформа ---
    PlatformInfo platform_info_;
    PlatformType platform_ = PlatformType::UNKNOWN;
    LocalOffset working_pos_;

    //--- Внешний контекст (модель) ---
    bool ctx_boarding_ = false;
    bool ctx_alighting_ = false;
    bool boarding_active_ = true;   ///< Посадка разрешена станцией

    //--- Потоки пассажиров ---
    bool alighting_request_ = false;
    bool boarding_request_ = false;
    int boarding_request_count_ = 0;
    bool alighting_started_ = false;
    bool boarding_started_ = false;

    //--- Опаздывающий пассажир (п.14 ТЗ) ---
    bool late_rolled_ = false;      ///< Бросок вероятности на остановке
    bool late_pending_ = false;
    double late_timer_ = 0.0;

    //--- Цикл остановки ---
    bool stop_active_ = false;      ///< Обслуживание остановки начато
    Emergency emergency_ = Emergency::NONE;
};

//------------------------------------------------------------------------------
//
//      Система проводников поезда
//
//------------------------------------------------------------------------------
class CONDUCTOR_EXPORT ConductorSystem
{
public:

    ConductorSystem() = default;

    ~ConductorSystem();

    ConductorSystem(const ConductorSystem&) = delete;
    ConductorSystem& operator=(const ConductorSystem&) = delete;

    /// Загрузка секции [Conductor] конфига поезда
    void loadConfig(const QString& train_cfg_path);

    /// Привязка состава: перепривязывает проводников из пула по
    /// пассажирским вагонам (п.1 ТЗ - по одному на вагон). Повторный
    /// вызов переиспользует объекты пула без аллокаций (сцепка/
    /// расцепка составов, ТЗ "Оптимизация" п.13); пул растёт только
    /// до максимального числа пассажирских вагонов
    void attachTrain(const std::vector<WagonInfo>& wagons);

    /// Метка поезда для журнала
    void setLabel(const QString& label);

    /// Перенос параметров другой системы (новому поезду при расцепке)
    void copyConfig(const ConductorSystem& other);

    /// Система включена секцией [Conductor]
    bool isEnabled() const;

    /// В составе есть проводники (пассажирские вагоны)
    bool hasConductors() const;

    //--- Внешний контекст (поток модели, до выдачи шага поездам) ---

    /// Поезд в зоне станции (остановка для обслуживания пассажиров)
    void setStationZone(bool in_zone);

    /// Посадка активна (разрешение станции на начало посадки)
    void setBoardingActive(bool active);

    /// Поезд движется (пишет поток поезда из Train::slotStep;
    /// читается на следующем тике вместе с остальным контекстом)
    void setTrainMoving(bool moving);

    /// Близость игрока: ACTIVE/INACTIVE проводников (п.19 ТЗ)
    void setPlayerNear(bool near);

    /// Данные платформы для вагона (п.5 ТЗ)
    void setWagonPlatform(size_t vehicle_idx, const PlatformInfo& info);

    /// Состояние пассажирских потоков вагона (п.16 ТЗ)
    void setWagonPassengerState(size_t vehicle_idx, bool boarding,
                                bool alighting);

    /// Команда машиниста REQUEST_DEPARTURE_READY (п.13 ТЗ):
    /// финальная ускоренная проверка
    void requestDepartureReady();

    //--- Шаг и агрегаты ---

    /// Шаг системы (поток поезда, из Train::slotStep)
    void step(double dt);

    /// Общая готовность поезда: логическое И готовностей всех
    /// проводников активного цикла обслуживания (п.11 ТЗ).
    /// Система выключена или цикл не начат - true
    bool isTrainReady() const;

    /// Проводник вагона по индексу ПЕ (nullptr - вагон без проводника)
    Conductor* getConductor(size_t vehicle_idx) const;

    /// Все проводники
    const std::vector<Conductor*>& getConductors() const;

    /// Статистика NPC-пула для профайлера (активные/спящие/пул)
    NpcStats getNpcStats() const;

    const Config& getConfig() const;

    /// Отладочная сводка (состояния всех проводников)
    QString getDebugMsg() const;

private:

    Config cfg_;
    QString label_ = "поезд";

    //--- Объектный пул (ТЗ "Оптимизация", п.13): фиксированный буфер
    //--- проводников, переиспользуемый между привязками без аллокаций.
    //--- conductors_ - активное подмножество пула (первые bound слотов)
    std::vector<Conductor*> pool_;
    std::vector<char> pool_used_;   ///< Слот хоть раз привязывался
    std::size_t pool_reused_ = 0;   ///< Суммарно переиспользований
    std::vector<Conductor*> conductors_;
    std::map<size_t, Conductor*> by_vehicle_;

    //--- Контекст шага (см. комментарий по потокам в шапке файла) ---
    bool station_zone_ = false;     ///< Пишет модель
    bool boarding_active_ = true;   ///< Пишет модель
    bool player_near_ = true;       ///< Пишет модель
    bool train_moving_ = true;      ///< Пишет поток поезда

    bool ready_reported_ = false;   ///< Сводка "поезд готов" в журнал
};

} // namespace conductor

#endif // CONDUCTOR_SYSTEM_H
