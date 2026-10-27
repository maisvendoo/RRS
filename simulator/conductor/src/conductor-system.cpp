//------------------------------------------------------------------------------
//
//      Conductors system (проводники пассажирского поезда)
//
//------------------------------------------------------------------------------

#include    "conductor-system.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

namespace conductor
{

namespace
{

/// Ожидание старта потока пассажиров, с: если за это время поток
/// не начался (вагон пуст / очередь пуста) - фаза завершается
constexpr double kFlowStartGrace = 10.0;

/// Пауза в служебном купе, с (появление / уход в DESPAWN)
constexpr double kCoupeTime = 2.0;

/// Выход из купе на площадку (WAITING_FOR_TRAIN), с
constexpr double kAppearTime = 3.0;

/// Стойка у вагона после получения отправления (DEPARTURE), с
constexpr double kDepartureTime = 2.0;

/// Минимальный отступ рабочей позиции от борта вагона, м (п.18 ТЗ)
constexpr double kMinLateralClearance = 0.3;

/// Кандидаты смещения рабочей позиции от борта, м (п.4, 18 ТЗ)
constexpr double kLateralCandidates[] = {0.8, 1.2, 1.6, 2.0};

/// Половина пи (без зависимости от _USE_MATH_DEFINES)
constexpr double kHalfPi = 1.5707963267948966;

/// Имена состояний для отладки
const char* stateName(State state)
{
    switch (state)
    {
    case State::DESPAWN:            return "DESPAWN";
    case State::SERVICE_COUPE:      return "SERVICE_COUPE";
    case State::WAITING_FOR_TRAIN:  return "WAITING_FOR_TRAIN";
    case State::GOING_TO_WAGON:     return "GOING_TO_WAGON";
    case State::WORKING:            return "WORKING";
    case State::READY:              return "READY";
    case State::DEPARTURE:          return "DEPARTURE";
    case State::RETURNING:          return "RETURNING";
    }

    return "UNKNOWN";
}

/// Имена фаз WORKING для отладки
const char* phaseName(WorkPhase phase)
{
    switch (phase)
    {
    case WorkPhase::NONE:               return "NONE";
    case WorkPhase::DEPLOYING_STAIRS:   return "DEPLOYING_STAIRS";
    case WorkPhase::OPENING_DOOR:       return "OPENING_DOOR";
    case WorkPhase::ALIGHTING:          return "ALIGHTING";
    case WorkPhase::BOARDING:           return "BOARDING";
    case WorkPhase::CHECKING:           return "CHECKING";
    case WorkPhase::CLOSING_DOOR:       return "CLOSING_DOOR";
    case WorkPhase::RETRACTING_STAIRS:  return "RETRACTING_STAIRS";
    case WorkPhase::STUCK:              return "STUCK";
    }

    return "UNKNOWN";
}

} // namespace

//------------------------------------------------------------------------------
//
//      Проводник
//
//------------------------------------------------------------------------------
Conductor::Conductor(const WagonInfo& wagon, const Config* cfg,
                     std::uint64_t seed)
    : wagon_(wagon),
      cfg_(cfg),
      seed_(seed),
      rng_(seed | 1u)
{
    // Расчёт на случай, если платформа известна уже на момент создания
    calcWorkingPosition();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::setLabel(const QString& label)
{
    label_ = label;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::rebind(const WagonInfo& wagon, std::uint64_t seed)
{
    // Полный сброс + привязка к новому вагону без аллокации (пул,
    // ТЗ "Оптимизация", п.13): проводник начинается с DESPAWN и ждёт
    // ближайшую остановку в зоне станции
    wagon_ = wagon;
    seed_ = seed;
    rng_ = seed | 1u;

    state_ = State::DESPAWN;
    work_phase_ = WorkPhase::NONE;
    after_open_phase_ = WorkPhase::ALIGHTING;
    state_timer_ = 0.0;
    phase_timer_ = 0.0;
    activity_ = NpcActivity::DESPAWN;
    hurry_ = false;
    despawn_next_ = false;
    stop_active_ = false;
    late_rolled_ = false;
    late_pending_ = false;
    late_timer_ = 0.0;

    platform_info_ = PlatformInfo{};
    emergency_ = Emergency::NONE;

    resetService();
    calcWorkingPosition();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::step(double dt, bool train_moving, bool in_station_zone,
                     bool player_near)
{
    // Активность NPC (п.19 ТЗ): DESPAWN - удалён из сцены,
    // INACTIVE - только логика, ACTIVE - полная симуляция
    activity_ = (state_ == State::DESPAWN) ? NpcActivity::DESPAWN
              : (player_near ? NpcActivity::ACTIVE : NpcActivity::INACTIVE);

    state_timer_ += dt;

    switch (state_)
    {
    case State::DESPAWN:
    {
        // Зона покинута - разблокируем спавн следующей остановки (п.2)
        if (!in_station_zone || train_moving)
            stop_active_ = false;

        // Появление перед началом посадки (п.3, 19 ТЗ)
        if (in_station_zone && !train_moving && !stop_active_)
        {
            stop_active_ = true;
            spawn();
        }

        break;
    }

    case State::SERVICE_COUPE:
    {
        // Внезапное отправление, пока проводник в купе - сразу DESPAWN
        if (train_moving || !in_station_zone)
        {
            resetService();
            state_ = State::DESPAWN;
            despawn_next_ = false;
            break;
        }

        if (state_timer_ >= kCoupeTime)
        {
            if (despawn_next_)
            {
                // Возврат завершён: NPC покидает сцену (п.2, 19 ТЗ)
                state_ = State::DESPAWN;
                state_timer_ = 0.0;
            }
            else
            {
                state_ = State::WAITING_FOR_TRAIN;
                state_timer_ = 0.0;
            }
        }

        break;
    }

    case State::WAITING_FOR_TRAIN:
    {
        if (train_moving || !in_station_zone)
        {
            // Поезд ушёл до начала обслуживания - назад в купе
            state_ = State::SERVICE_COUPE;
            state_timer_ = 0.0;
            despawn_next_ = true;
            break;
        }

        if (state_timer_ >= kAppearTime)
        {
            state_ = State::GOING_TO_WAGON;
            state_timer_ = 0.0;
            Journal::instance()->info(logPrefix() +
                    " проводник идёт к вагону (GOING_TO_WAGON)");
        }

        break;
    }

    case State::GOING_TO_WAGON:
    {
        if (train_moving || !in_station_zone)
        {
            beginDeparture();
            break;
        }

        if (state_timer_ >= cfg_->walk_time)
        {
            // Подошёл: определяет платформу -> лестница -> дверь
            // (п.9 шаги 5-8 ТЗ)
            determinePlatform();
            beginWorking();
        }

        break;
    }

    case State::WORKING:
    {
        if (train_moving)
        {
            beginDeparture();
            break;
        }

        stepWorking(dt);
        break;
    }

    case State::READY:
    {
        if (train_moving)
        {
            beginDeparture();
            break;
        }

        stepReady(dt);
        break;
    }

    case State::DEPARTURE:
    {
        if (state_timer_ >= kDepartureTime)
        {
            state_ = State::RETURNING;
            state_timer_ = 0.0;
        }

        break;
    }

    case State::RETURNING:
    {
        if (state_timer_ >= cfg_->return_time)
        {
            // Вернулся в служебное купе: сброс и DESPAWN (п.2, 19 ТЗ)
            resetService();
            state_ = State::SERVICE_COUPE;
            state_timer_ = 0.0;
            despawn_next_ = true;
        }

        break;
    }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
State Conductor::getState() const
{
    return state_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WorkPhase Conductor::getWorkPhase() const
{
    return work_phase_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DoorState Conductor::getDoorState() const
{
    return door_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StairsState Conductor::getStairsState() const
{
    return stairs_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PlatformType Conductor::getPlatformType() const
{
    return platform_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Flag Conductor::getFlag() const
{
    // Жёлтый флаг (п.10 ТЗ): посадка завершена, дверь закрыта, лестница
    // убрана, проверка пройдена
    if (state_ == State::READY && emergency_ == Emergency::NONE &&
            door_ == DoorState::CLOSED && stairs_ == StairsState::RETRACTED)
        return Flag::YELLOW;

    // Красный флаг: проводник у вагона, вагон ещё не готов
    if (state_ == State::WORKING || state_ == State::READY)
        return Flag::RED;

    // Идёт/возвращается/в купе/удалён - флага нет
    return Flag::NONE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Emergency Conductor::getEmergency() const
{
    return emergency_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
NpcActivity Conductor::getActivity() const
{
    return activity_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Conductor::isReady() const
{
    return getFlag() == Flag::YELLOW;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocalOffset Conductor::getServiceCoupeOffset() const
{
    return wagon_.coupe_offset;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocalOffset Conductor::getDoorSocketOffset() const
{
    return wagon_.door_socket;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocalOffset Conductor::getWorkingPositionOffset() const
{
    return working_pos_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocalOffset Conductor::getCurrentPositionOffset() const
{
    const LocalOffset& coupe = wagon_.coupe_offset;

    switch (state_)
    {
    case State::GOING_TO_WAGON:
    {
        // INACTIVE: без интерполяции - мгновенно у цели (п.19 ТЗ)
        if (activity_ != NpcActivity::ACTIVE)
            return working_pos_;

        const double k = std::min(state_timer_ / std::max(cfg_->walk_time,
                                                          1.0), 1.0);

        return LocalOffset{coupe.x + (working_pos_.x - coupe.x) * k,
                           coupe.y + (working_pos_.y - coupe.y) * k,
                           coupe.z + (working_pos_.z - coupe.z) * k};
    }

    case State::RETURNING:
    {
        if (activity_ != NpcActivity::ACTIVE)
            return coupe;

        const double k = std::min(state_timer_ / std::max(cfg_->return_time,
                                                          1.0), 1.0);

        return LocalOffset{working_pos_.x + (coupe.x - working_pos_.x) * k,
                           working_pos_.y + (coupe.y - working_pos_.y) * k,
                           working_pos_.z + (coupe.z - working_pos_.z) * k};
    }

    case State::WORKING:
    case State::READY:
    case State::DEPARTURE:
        return working_pos_;

    default:
        return coupe;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Conductor::getHeading() const
{
    switch (state_)
    {
    case State::GOING_TO_WAGON:
        return std::atan2(working_pos_.y - wagon_.coupe_offset.y,
                          working_pos_.x - wagon_.coupe_offset.x);

    case State::WORKING:
    case State::READY:
    case State::DEPARTURE:
        // Стоит у двери лицом к платформе
        return (platform_info_.side >= 0) ? kHalfPi : -kHalfPi;

    case State::RETURNING:
        return std::atan2(wagon_.coupe_offset.y - working_pos_.y,
                          wagon_.coupe_offset.x - working_pos_.x);

    default:
        return 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::setPlatform(const PlatformInfo& info)
{
    if (platform_info_.known == info.known &&
            std::abs(platform_info_.height_above_rail -
                     info.height_above_rail) < 1e-6 &&
            platform_info_.side == info.side)
        return;

    platform_info_ = info;

    // Рабочая позиция зависит от высоты поверхности (п.4 ТЗ)
    calcWorkingPosition();

    // Тип платформы пересчитывается на месте работы (п.5 ТЗ)
    if (state_ == State::WORKING || state_ == State::READY)
        determinePlatform();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::setPassengerState(bool boarding, bool alighting)
{
    ctx_boarding_ = boarding;
    ctx_alighting_ = alighting;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::setBoardingActive(bool active)
{
    boarding_active_ = active;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::setHurry(bool hurry)
{
    hurry_ = hurry;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Conductor::fetchBeginAlightingRequest()
{
    if (!alighting_request_)
        return false;

    alighting_request_ = false;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Conductor::fetchBeginBoardingRequest(int& out_count)
{
    if (!boarding_request_)
        return false;

    boarding_request_ = false;
    out_count = boarding_request_count_;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::markAlightingDelivered(int count)
{
    // Поток действительно запущен: ждём его завершения (п.16 ТЗ)
    if (count > 0)
        alighting_started_ = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::markBoardingDelivered(int count)
{
    if (count > 0)
        boarding_started_ = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Conductor::getVehicleIdx() const
{
    return wagon_.vehicle_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::uint64_t Conductor::getSeed() const
{
    return seed_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::spawn()
{
    // Спавн из служебного купе перед началом посадки (п.3, 19 ТЗ)
    state_ = State::SERVICE_COUPE;
    state_timer_ = 0.0;
    phase_timer_ = 0.0;
    despawn_next_ = false;
    work_phase_ = WorkPhase::NONE;
    door_ = DoorState::CLOSED;
    stairs_ = StairsState::RETRACTED;
    emergency_ = Emergency::NONE;
    stairs_used_ = false;
    close_attempts_ = 0;
    late_rolled_ = false;
    late_pending_ = false;
    alighting_request_ = false;
    boarding_request_ = false;
    alighting_started_ = false;
    boarding_started_ = false;

    Journal::instance()->info(logPrefix() +
            " проводник приступил к работе (остановка поезда)");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::determinePlatform()
{
    // Тип платформы - ТОЛЬКО по разности высот поверхности и головки
    // рельса, не по имени объекта (п.5 ТЗ):
    // HeightDifference = PlatformHeight - RailHeight
    const double diff = platform_info_.known
            ? platform_info_.height_above_rail
            : 0.0;

    platform_ = (diff >= cfg_->platform_high_threshold)
            ? PlatformType::HIGH
            : PlatformType::LOW;

    Journal::instance()->info(logPrefix() +
            QString(" платформа: высота %1 м над ГР, порог %2 м -> %3")
                .arg(diff, 0, 'f', 2)
                .arg(cfg_->platform_high_threshold, 0, 'f', 2)
                .arg(platform_ == PlatformType::HIGH ? "ВЫСОКАЯ"
                                                     : "НИЗКАЯ"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::beginWorking()
{
    state_ = State::WORKING;
    state_timer_ = 0.0;
    phase_timer_ = 0.0;
    close_attempts_ = 0;
    emergency_ = Emergency::NONE;

    // Высокая -> дверь; низкая + лестница -> лестница -> дверь (п.6 ТЗ)
    if (platform_ == PlatformType::LOW && wagon_.has_stairs)
    {
        work_phase_ = WorkPhase::DEPLOYING_STAIRS;
        stairs_ = StairsState::DEPLOYING;
        stairs_used_ = true;
        after_open_phase_ = WorkPhase::ALIGHTING;
        Journal::instance()->info(logPrefix() +
                " опускает лестницу (низкая платформа)");
    }
    else
    {
        work_phase_ = WorkPhase::OPENING_DOOR;
        door_ = DoorState::CLOSED;
        after_open_phase_ = WorkPhase::ALIGHTING;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::stepWorking(double dt)
{
    phase_timer_ += dt;

    switch (work_phase_)
    {
    case WorkPhase::DEPLOYING_STAIRS:
    {
        // Ждём завершения "анимации" лестницы (п.6 ТЗ).
        // after_open_phase_ задан тем, кто вошёл в фазу лестницы:
        // обычный цикл - ALIGHTING, опаздывающий - BOARDING
        if (phase_timer_ >= cfg_->stairs_deploy_time)
        {
            stairs_ = StairsState::DEPLOYED;
            work_phase_ = WorkPhase::OPENING_DOOR;
            phase_timer_ = 0.0;
            Journal::instance()->info(logPrefix() + " лестница разложена");
        }

        break;
    }

    case WorkPhase::OPENING_DOOR:
    {
        if (phase_timer_ >= cfg_->door_open_time)
        {
            door_ = DoorState::OPEN;
            phase_timer_ = 0.0;

            switch (after_open_phase_)
            {
            // Первый заход: высадка (п.9 шаг 9, п.16 ТЗ)
            case WorkPhase::ALIGHTING:
                work_phase_ = WorkPhase::ALIGHTING;
                alighting_request_ = true;
                alighting_started_ = false;
                Journal::instance()->info(logPrefix() +
                        " дверь открыта, высадка пассажиров");
                break;

            // Опаздывающий пассажир: сразу посадка (п.14 ТЗ)
            case WorkPhase::BOARDING:
                work_phase_ = WorkPhase::BOARDING;
                boarding_request_ = true;
                boarding_started_ = false;
                break;

            // Повтор закрытия после блокировки (п.17 ТЗ)
            case WorkPhase::CLOSING_DOOR:
            default:
                enterClosingDoor();
                break;
            }
        }

        break;
    }

    case WorkPhase::ALIGHTING:
    {
        if (ctx_alighting_)
            alighting_started_ = true;

        // Пассажиры ещё выходят - дверь не закрывается (п.16 ТЗ).
        // Grace покрывает случай пустого вагона
        if (!ctx_alighting_ && (alighting_started_ ||
                                phase_timer_ >= kFlowStartGrace))
        {
            // Посадку станция ещё не открыла - ждём с открытой дверью
            if (!boarding_active_)
                break;

            work_phase_ = WorkPhase::BOARDING;
            phase_timer_ = 0.0;
            boarding_request_ = true;
            boarding_request_count_ = 0;
            boarding_started_ = false;
            Journal::instance()->info(logPrefix() +
                    " высадка завершена, посадка пассажиров");
        }

        break;
    }

    case WorkPhase::BOARDING:
    {
        if (ctx_boarding_)
            boarding_started_ = true;

        // Поток ещё не начался и grace не истёк - ждём у двери
        // (очередь могла быть пуста или перенаправлена)
        if (!boarding_started_ && phase_timer_ < kFlowStartGrace)
            break;

        // Посадка завершена (п.9 шаг 11 ТЗ) - проверка готовности
        if (!ctx_boarding_)
        {
            work_phase_ = WorkPhase::CHECKING;
            phase_timer_ = 0.0;

            if (emergency_ == Emergency::LATE_PASSENGER)
                Journal::instance()->info(logPrefix() +
                        " опаздывающий пассажир сел в вагон");
        }

        break;
    }

    case WorkPhase::CHECKING:
    {
        // Пассажиры возобновили поток - внештатная ситуация (п.15 ТЗ):
        // READY -> NOT_READY, после устранения -> CHECKING -> READY
        if (ctx_boarding_ || ctx_alighting_)
        {
            if (emergency_ == Emergency::NONE)
            {
                emergency_ = Emergency::PASSENGER_STILL_BOARDING;
                Journal::instance()->warning(logPrefix() +
                        " пассажиры ещё садятся - готовность снята");
            }

            work_phase_ = WorkPhase::BOARDING;
            phase_timer_ = 0.0;
            boarding_request_ = false;
            boarding_request_count_ = 0;
            boarding_started_ = false;
            break;
        }

        const double check_time = hurry_ ? cfg_->check_time / 2.0
                                         : cfg_->check_time;

        if (phase_timer_ >= check_time)
        {
            // Закрытие двери (п.9 шаг 13 ТЗ)
            enterClosingDoor();
        }

        break;
    }

    case WorkPhase::CLOSING_DOOR:
    {
        if (phase_timer_ >= cfg_->door_close_time)
        {
            // Случайная блокировка двери пассажиром (п.15, 17 ТЗ)
            if (nextUniform() < cfg_->door_blocked_probability)
            {
                door_ = DoorState::BLOCKED;
                emergency_ = Emergency::PASSENGER_BLOCKING_DOOR;

                Journal::instance()->warning(logPrefix() +
                        " пассажир блокирует дверь - повторное закрытие");

                // Исчерпаны попытки: неисправность двери (п.17 ТЗ) -
                // красный флаг до конца остановки
                if (close_attempts_ >=
                        std::max(cfg_->door_not_closed_retries, 1))
                {
                    emergency_ = Emergency::DOOR_NOT_CLOSED;
                    work_phase_ = WorkPhase::STUCK;
                    Journal::instance()->error(logPrefix() +
                            " НЕИСПРАВНОСТЬ: дверь не закрывается, "
                            "жёлтый флаг не будет показан");
                    break;
                }

                // Повтор: открыть и снова закрыть
                work_phase_ = WorkPhase::OPENING_DOOR;
                after_open_phase_ = WorkPhase::CLOSING_DOOR;
                phase_timer_ = 0.0;
                break;
            }

            door_ = DoorState::CLOSED;

            if (emergency_ != Emergency::NONE)
            {
                // Проблема устранена: NOT_READY -> CHECKING -> READY
                Journal::instance()->info(logPrefix() +
                        " проблема устранена, дверь закрыта");
                emergency_ = Emergency::NONE;
            }
            else
                Journal::instance()->info(logPrefix() + " дверь закрыта");

            // Убирает лестницу, если она использовалась (п.9 шаг 14)
            if (stairs_used_ && stairs_ == StairsState::DEPLOYED)
            {
                work_phase_ = WorkPhase::RETRACTING_STAIRS;
                stairs_ = StairsState::RETRACTING;
                phase_timer_ = 0.0;
            }
            else
                finishWorking();
        }

        break;
    }

    case WorkPhase::RETRACTING_STAIRS:
    {
        if (phase_timer_ >= std::max(cfg_->stairs_deploy_time, 0.5))
        {
            stairs_ = StairsState::RETRACTED;
            stairs_used_ = false;
            Journal::instance()->info(logPrefix() + " лестница убрана");
            finishWorking();
        }

        break;
    }

    case WorkPhase::STUCK:
    {
        // Неисправность: ждём отправления (п.17 ТЗ)
        break;
    }

    default:
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::enterClosingDoor()
{
    work_phase_ = WorkPhase::CLOSING_DOOR;
    door_ = DoorState::CLOSING;
    phase_timer_ = 0.0;
    ++close_attempts_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::stepReady(double dt)
{
    // Опаздывающий пассажир (п.14 ТЗ): жёлтый флаг уже показан,
    // пассажир бежит к вагону -> задержка -> возврат к посадке
    if (late_pending_)
    {
        late_timer_ -= dt;

        if (late_timer_ <= 0.0)
        {
            late_pending_ = false;
            triggerLatePassenger();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::finishWorking()
{
    state_ = State::READY;
    state_timer_ = 0.0;
    work_phase_ = WorkPhase::NONE;

    // Один бросок вероятности на остановку, детерминированный RNG
    // на seed вагона (п.14 ТЗ)
    if (!late_rolled_)
    {
        late_rolled_ = true;

        if (nextUniform() < cfg_->late_passenger_probability)
            late_timer_ = cfg_->late_passenger_delay *
                    (0.5 + 0.5 * nextUniform());
        else
            late_timer_ = 0.0;

        late_pending_ = late_timer_ > 0.0;
    }

    Journal::instance()->info(logPrefix() +
            " ЖЁЛТЫЙ ФЛАГ: вагон готов к отправлению");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::triggerLatePassenger()
{
    // Готов -> опаздывающий замечен -> посадка -> повторная проверка
    // -> жёлтый флаг (п.14 ТЗ)
    state_ = State::WORKING;
    state_timer_ = 0.0;
    phase_timer_ = 0.0;
    emergency_ = Emergency::LATE_PASSENGER;
    boarding_request_count_ = 1;

    Journal::instance()->warning(logPrefix() +
            " опаздывающий пассажир бежит к вагону - отправление "
            "задерживается");

    // На низкой платформе лестница нужна снова (п.6 ТЗ)
    if (platform_ == PlatformType::LOW && wagon_.has_stairs)
    {
        work_phase_ = WorkPhase::DEPLOYING_STAIRS;
        stairs_ = StairsState::DEPLOYING;
        stairs_used_ = true;
        after_open_phase_ = WorkPhase::BOARDING;
    }
    else
    {
        work_phase_ = WorkPhase::OPENING_DOOR;
        after_open_phase_ = WorkPhase::BOARDING;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::beginDeparture()
{
    // Подтверждение отправления (п.9 шаг 16): дверь принудительно
    // закрыта, лестница убирается по ходу возврата
    state_ = State::DEPARTURE;
    state_timer_ = 0.0;
    work_phase_ = WorkPhase::NONE;
    door_ = DoorState::CLOSED;

    if (stairs_ != StairsState::RETRACTED)
        stairs_ = StairsState::RETRACTING;

    emergency_ = Emergency::NONE;
    late_pending_ = false;
    alighting_request_ = false;
    boarding_request_ = false;

    Journal::instance()->info(logPrefix() +
            " получено отправление, возврат в служебное купе");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::resetService()
{
    // Полный сброс к исходному состоянию (п.2, 19 ТЗ)
    work_phase_ = WorkPhase::NONE;
    door_ = DoorState::CLOSED;
    stairs_ = StairsState::RETRACTED;
    stairs_used_ = false;
    close_attempts_ = 0;
    emergency_ = Emergency::NONE;
    late_rolled_ = false;
    late_pending_ = false;
    alighting_request_ = false;
    boarding_request_ = false;
    alighting_started_ = false;
    boarding_started_ = false;
    ctx_boarding_ = false;
    ctx_alighting_ = false;
    state_timer_ = 0.0;
    phase_timer_ = 0.0;
    platform_ = PlatformType::UNKNOWN;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Conductor::calcWorkingPosition()
{
    // Рабочая позиция относительно вагона: DoorSocket + смещение
    // наружу от борта (п.3-4 ТЗ). Проверка PositionValid (п.18 ТЗ):
    // позиция недействительна внутри габарита вагона - берём
    // ближайший свободный вариант у двери
    const double half_width = wagon_.width / 2.0;
    const int side = (platform_info_.side >= 0) ? 1 : -1;

    working_pos_.x = wagon_.door_socket.x;

    bool valid = false;

    for (const double off : kLateralCandidates)
    {
        const double y = side * (half_width + off);

        // Вне габарита вагона с запасом и в разумных пределах
        // от борта (не внутри вагона, не внутри платформы)
        const bool ok = (std::abs(y) > half_width + kMinLateralClearance) &&
                        (off >= 0.5) && (off <= 2.5);

        if (ok)
        {
            working_pos_.y = y;
            valid = true;
            break;
        }
    }

    if (!valid)
        working_pos_.y = side * (half_width + 1.2);

    // Проводник стоит на поверхности платформы (высокая) либо на
    // уровне земли у лестницы (низкая): z = высота поверхности
    working_pos_.z = std::max(platform_info_.height_above_rail, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::uint64_t Conductor::nextRand()
{
    // SplitMix64: детерминирован на всех платформах/стандартных
    // библиотеках (в отличие от распределений <random>)
    rng_ += 0x9E3779B97F4A7C15ull;

    std::uint64_t z = rng_;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;

    return z ^ (z >> 31);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Conductor::nextUniform()
{
    return static_cast<double>(nextRand() >> 11) *
            (1.0 / 9007199254740992.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Conductor::logPrefix() const
{
    return QString("[CONDUCTOR] %1, вагон %2:").arg(label_).arg(
                static_cast<qulonglong>(wagon_.vehicle_idx));
}

//------------------------------------------------------------------------------
//
//      Система проводников поезда
//
//------------------------------------------------------------------------------
ConductorSystem::~ConductorSystem()
{
    // Все проводники живут в пуле; conductors_ - лишь подмножество
    // его слотов, отдельного удаления не требует
    for (auto conductor : pool_)
        delete conductor;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::loadConfig(const QString& train_cfg_path)
{
    CfgReader cfg;

    if (!cfg.load(train_cfg_path))
    {
        // Секция [Conductor] не найдена - работаем на дефолтах
        Journal::instance()->info(
                    "[CONDUCTOR] " + label_ +
                    ": конфиг не найден, параметры по умолчанию");
        return;
    }

    cfg.getBool("Conductor", "Enabled", cfg_.enabled);
    cfg.getDouble("Conductor", "PlatformHighThreshold",
                  cfg_.platform_high_threshold);
    cfg.getDouble("Conductor", "StairsDeployTime", cfg_.stairs_deploy_time);
    cfg.getDouble("Conductor", "LatePassengerProbability",
                  cfg_.late_passenger_probability);
    cfg.getDouble("Conductor", "LatePassengerDelay",
                  cfg_.late_passenger_delay);
    cfg.getDouble("Conductor", "WalkTime", cfg_.walk_time);
    cfg.getDouble("Conductor", "ReturnTime", cfg_.return_time);
    cfg.getDouble("Conductor", "CheckTime", cfg_.check_time);
    cfg.getDouble("Conductor", "DoorOpenTime", cfg_.door_open_time);
    cfg.getDouble("Conductor", "DoorCloseTime", cfg_.door_close_time);
    cfg.getDouble("Conductor", "DoorBlockedProbability",
                  cfg_.door_blocked_probability);

    int retries = cfg_.door_not_closed_retries;
    if (cfg.getInt("Conductor", "DoorNotClosedRetries", retries))
        cfg_.door_not_closed_retries = retries;

    int seed = 0;
    if (cfg.getInt("Conductor", "Seed", seed))
        cfg_.seed = static_cast<std::uint64_t>(
                    static_cast<long long>(seed));

    // Ограничения разумного (п.14: "не создавать постоянные задержки")
    cfg_.platform_high_threshold = std::max(cfg_.platform_high_threshold,
                                            0.1);
    cfg_.stairs_deploy_time = std::max(cfg_.stairs_deploy_time, 0.5);
    cfg_.late_passenger_probability =
            std::min(std::max(cfg_.late_passenger_probability, 0.0), 1.0);
    cfg_.late_passenger_delay = std::max(cfg_.late_passenger_delay, 0.0);
    cfg_.walk_time = std::max(cfg_.walk_time, 1.0);
    cfg_.return_time = std::max(cfg_.return_time, 1.0);
    cfg_.check_time = std::max(cfg_.check_time, 0.5);
    cfg_.door_open_time = std::max(cfg_.door_open_time, 0.5);
    cfg_.door_close_time = std::max(cfg_.door_close_time, 0.5);
    cfg_.door_blocked_probability =
            std::min(std::max(cfg_.door_blocked_probability, 0.0), 1.0);
    cfg_.door_not_closed_retries = std::max(cfg_.door_not_closed_retries, 1);

    Journal::instance()->info(QString(
                "[CONDUCTOR] %1: конфиг загружен (Enabled=%2, "
                "порог платформы %3 м, лестница %4 c, "
                "опаздывающий p=%5 delay=%6 c)")
            .arg(label_)
            .arg(cfg_.enabled ? "да" : "нет")
            .arg(cfg_.platform_high_threshold, 0, 'f', 2)
            .arg(cfg_.stairs_deploy_time, 0, 'f', 1)
            .arg(cfg_.late_passenger_probability, 0, 'f', 2)
            .arg(cfg_.late_passenger_delay, 0, 'f', 1));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::attachTrain(const std::vector<WagonInfo>& wagons)
{
    // Пассажирские вагоны: по одному проводнику на вагон (п.1 ТЗ)
    std::vector<const WagonInfo*> passenger;

    for (const WagonInfo& wagon : wagons)
    {
        if (wagon.passenger)
            passenger.push_back(&wagon);
    }

    // Объектный пул (ТЗ "Оптимизация", п.13): буфер растёт только до
    // максимального числа пассажирских вагонов поезда и дальше
    // переиспользуется между остановками/пересцепками без аллокаций
    while (pool_.size() < passenger.size())
    {
        pool_.push_back(new Conductor(WagonInfo{}, &cfg_, 1));
        pool_used_.push_back(0);
    }

    conductors_.clear();
    by_vehicle_.clear();

    std::size_t reused_now = 0;

    for (std::size_t i = 0; i < passenger.size(); ++i)
    {
        Conductor* conductor = pool_[i];

        // Слот уже работал - это переиспользование пула
        if (pool_used_[i] != 0)
            ++reused_now;

        pool_used_[i] = 1;

        // Seed вагона: детерминированный RNG (п.14 ТЗ)
        const std::uint64_t seed = cfg_.seed ^
                (0x9E3779B97F4A7C15ull *
                 static_cast<std::uint64_t>(passenger[i]->vehicle_idx + 1));

        conductor->rebind(*passenger[i], seed);
        conductor->setLabel(label_);

        conductors_.push_back(conductor);
        by_vehicle_[passenger[i]->vehicle_idx] = conductor;
    }

    pool_reused_ += reused_now;

    Journal::instance()->info(QString(
                "[CONDUCTOR] %1: привязано проводников: %2 "
                "(пул %3, переиспользовано %4)")
            .arg(label_)
            .arg(static_cast<int>(conductors_.size()))
            .arg(static_cast<int>(pool_.size()))
            .arg(static_cast<int>(reused_now)));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setLabel(const QString& label)
{
    label_ = label;

    for (auto conductor : conductors_)
        conductor->setLabel(label);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::copyConfig(const ConductorSystem& other)
{
    // Проводники хранят указатель на cfg_ этой системы, поэтому
    // присваивание поля (не замена объекта) обновляет их параметры
    cfg_ = other.cfg_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ConductorSystem::isEnabled() const
{
    return cfg_.enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ConductorSystem::hasConductors() const
{
    return !conductors_.empty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setStationZone(bool in_zone)
{
    station_zone_ = in_zone;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setBoardingActive(bool active)
{
    boarding_active_ = active;

    for (auto conductor : conductors_)
        conductor->setBoardingActive(active);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setPlayerNear(bool near)
{
    player_near_ = near;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setTrainMoving(bool moving)
{
    train_moving_ = moving;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setWagonPlatform(size_t vehicle_idx,
                                       const PlatformInfo& info)
{
    Conductor* conductor = getConductor(vehicle_idx);

    if (conductor != nullptr)
        conductor->setPlatform(info);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::setWagonPassengerState(size_t vehicle_idx,
                                             bool boarding, bool alighting)
{
    Conductor* conductor = getConductor(vehicle_idx);

    if (conductor != nullptr)
        conductor->setPassengerState(boarding, alighting);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::requestDepartureReady()
{
    // Команда машиниста REQUEST_DEPARTURE_READY (п.13 ТЗ):
    // финальная проверка без автоматического отправления
    for (auto conductor : conductors_)
        conductor->setHurry(true);

    Journal::instance()->info("[CONDUCTOR] " + label_ +
            ": команда машиниста - финальная проверка вагонов");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConductorSystem::step(double dt)
{
    if (!cfg_.enabled || conductors_.empty())
        return;

    for (auto conductor : conductors_)
        conductor->step(dt, train_moving_, station_zone_, player_near_);

    // Сводка общей готовности (п.11 ТЗ)
    if (isTrainReady() && !ready_reported_)
    {
        bool in_cycle = false;

        for (auto conductor : conductors_)
            if (conductor->getState() != State::DESPAWN)
                in_cycle = true;

        if (in_cycle)
        {
            ready_reported_ = true;
            Journal::instance()->info("[CONDUCTOR] " + label_ +
                    ": ВСЕ проводники готовы - поезд может отправляться");
        }
    }
    else if (!isTrainReady())
        ready_reported_ = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ConductorSystem::isTrainReady() const
{
    // Система выключена или проводников нет - поезд не ждём (п.11 ТЗ)
    if (!cfg_.enabled || conductors_.empty())
        return true;

    // Хотя бы один NOT_READY - поезд неготов. Проводники вне цикла
    // обслуживания (DESPAWN) не блокируют готовность
    for (auto conductor : conductors_)
    {
        if (conductor->getState() == State::DESPAWN)
            continue;

        if (!conductor->isReady())
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Conductor* ConductorSystem::getConductor(size_t vehicle_idx) const
{
    const auto it = by_vehicle_.find(vehicle_idx);

    return (it != by_vehicle_.end()) ? it->second : nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::vector<Conductor*>& ConductorSystem::getConductors() const
{
    return conductors_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
NpcStats ConductorSystem::getNpcStats() const
{
    // Статистика пула для профайлера (ТЗ "Оптимизация", п.13):
    // размер пула, привязанные объекты, активные/спящие/удалённые NPC
    // и накопленное число переиспользований
    NpcStats stats;

    stats.pool_size = pool_.size();
    stats.bound = conductors_.size();
    stats.reused = pool_reused_;

    for (const Conductor* conductor : conductors_)
    {
        switch (conductor->getActivity())
        {
        case NpcActivity::ACTIVE:
            ++stats.active;
            break;

        case NpcActivity::INACTIVE:
            ++stats.inactive;
            break;

        case NpcActivity::DESPAWN:
            ++stats.despawned;
            break;
        }
    }

    return stats;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const Config& ConductorSystem::getConfig() const
{
    return cfg_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ConductorSystem::getDebugMsg() const
{
    QString msg = QString("[CONDUCTOR] %1: готов=%2, проводников=%3 [")
            .arg(label_)
            .arg(isTrainReady() ? "ДА" : "НЕТ")
            .arg(static_cast<int>(conductors_.size()));

    for (size_t i = 0; i < conductors_.size(); ++i)
    {
        if (i > 0)
            msg += ", ";

        msg += QString("#%1 %2/%3 %4")
                .arg(static_cast<qulonglong>(
                         conductors_[i]->getVehicleIdx()))
                .arg(stateName(conductors_[i]->getState()))
                .arg(phaseName(conductors_[i]->getWorkPhase()))
                .arg(conductors_[i]->isReady() ? "Ж" : "К");
    }

    return msg + "]";
}

} // namespace conductor
