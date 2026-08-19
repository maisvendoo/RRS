//------------------------------------------------------------------------------
//
//      Service system (снабжение локомотива)
//
//------------------------------------------------------------------------------

#include    "vehicle-service.h"

#include    "vehicle-diesel.h"
#include    "vehicle-sand.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ServiceSystem::ServiceSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ServiceSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Service";

    bool has_enabled = cfg.getBool(sec, "Enabled", enabled);
    bool any_key = has_enabled;
    any_key = cfg.getDouble(sec, "FuelRate", fuel_rate) || any_key;
    cfg.getDouble(sec, "OilRate", oil_rate);
    cfg.getDouble(sec, "CoolantRate", coolant_rate);
    cfg.getDouble(sec, "SandRate", sand_rate);
    cfg.getDouble(sec, "MoveThreshold", move_threshold);

    // Секция есть: снабжение доступно, если не выключено явно
    if (any_key && !has_enabled)
        enabled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ServiceSystem::connectService(const QString& resource)
{
    if (!enabled)
    {
        last_error = "Service system disabled";
        return false;
    }

    // Повторное подключение после отмены/завершения разрешено (ТЗ, п.10)
    if (state == State::Connected || state == State::Refueling)
    {
        last_error = "Hose already connected";
        return false;
    }

    // Подключение только на стоянке (ТЗ, п.9, 11)
    if (std::abs(last_velocity) >= move_threshold)
    {
        last_error = "Cannot connect while moving";
        Journal::instance()->warning(
            "[SERVICE] Connect rejected: vehicle is moving");
        return false;
    }

    // Подключение только в зоне заправочной колонки (ТЗ, п.7-8)
    if (!in_zone)
    {
        last_error = "Vehicle is not in service zone";
        Journal::instance()->warning(
            "[SERVICE] Connect rejected: no service column here");
        return false;
    }

    const QString name = resource.toLower();

    if (name == "fuel")
        this->resource = Resource::Fuel;
    else if (name == "oil")
        this->resource = Resource::Oil;
    else if (name == "coolant")
        this->resource = Resource::Coolant;
    else if (name == "sand")
        this->resource = Resource::Sand;
    else
    {
        last_error = "Unknown resource: " + resource;
        Journal::instance()->warning(QString(
            "[SERVICE] Unknown resource: %1").arg(resource));
        return false;
    }

    state = State::Connected;
    initial_level = -1.0;   // уточняется на первом шаге (нужен уровень)
    delivered = 0.0;
    last_error = "";

    Journal::instance()->info(QString(
        "[SERVICE] Hose connected (%1), refueling will start")
        .arg(getResourceString()));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ServiceSystem::disconnectService()
{
    if (state == State::Connected || state == State::Refueling)
    {
        Journal::instance()->info(QString(
            "[SERVICE] Hose disconnected (%1), delivered %2")
            .arg(getResourceString())
            .arg(delivered, 0, 'f', 1));
    }

    state = State::Disconnected;
    delivered = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ServiceSystem::setInZone(bool in_zone_)
{
    in_zone = in_zone_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ServiceSystem::step(double dt,
                         double velocity,
                         DieselEngineSystem& diesel,
                         SandSystem& sand)
{
    if (dt <= 0.0)
        return;

    last_velocity = velocity;

    if (state != State::Connected && state != State::Refueling)
        return;

    // Движение при подключении: обрыв рукава, отмена операции (ТЗ, п.11)
    if (std::abs(velocity) >= move_threshold)
    {
        state = State::Aborted;
        last_error = "Hose torn off: movement during service";

        Journal::instance()->critical(QString(
            "[SERVICE] HOSE TORN OFF - vehicle moved during refueling "
            "(%1), operation CANCELLED")
            .arg(getResourceString()));

        delivered = 0.0;
        return;
    }

    // Выезд из зоны заправки с подключённым рукавом - обрыв (ТЗ, п.8)
    if (!in_zone)
    {
        state = State::Aborted;
        last_error = "Hose torn off: left the service zone";

        Journal::instance()->critical(QString(
            "[SERVICE] HOSE TORN OFF - vehicle left the service zone "
            "(%1), operation CANCELLED")
            .arg(getResourceString()));

        delivered = 0.0;
        return;
    }

    // Ресурс должен быть доступен на этой ПЕ (например, песок у вагона)
    if (!resourceAvailable(resource, diesel, sand))
    {
        state = State::Aborted;
        last_error = "Resource not available on this vehicle";

        Journal::instance()->critical(QString(
            "[SERVICE] Refueling CANCELLED: no %1 system on this vehicle")
            .arg(getResourceString()));

        return;
    }

    const double level = currentLevel(diesel, sand);
    last_level = level;

    // База прогресса фиксируется на первом шаге после подключения
    if (initial_level < 0.0)
        initial_level = level;

    // Стакан уже полон: подача не требуется
    if (level >= 0.999)
    {
        state = State::Completed;

        Journal::instance()->info(QString(
            "[SERVICE] %1 tank is FULL - nothing to fill")
            .arg(getResourceString()));

        return;
    }

    // Рукав подключён и проверен: начинается подача (ТЗ, п.10)
    if (state == State::Connected)
    {
        state = State::Refueling;

        Journal::instance()->info(QString(
            "[SERVICE] Refueling started (%1), rate %2/min")
            .arg(getResourceString())
            .arg(resource == Resource::Sand ? sand_rate :
                 resource == Resource::Fuel ? fuel_rate :
                 resource == Resource::Oil ? oil_rate : coolant_rate,
                 0, 'f', 0));
    }

    // Порционная подача: литраж за шаг (ТЗ, п.3: не мгновенно)
    const double rate_per_sec = (resource == Resource::Sand) ? sand_rate :
                             (resource == Resource::Fuel) ? fuel_rate :
                             (resource == Resource::Oil) ? oil_rate :
                                                           coolant_rate;

    const double portion = std::max(0.0, rate_per_sec / 60.0 * dt);

    const double before = currentLevel(diesel, sand);

    deliverPortion(portion, diesel, sand);
    delivered += portion;

    const double after = currentLevel(diesel, sand);

    // Автоматическая остановка при заполнении (ТЗ, п.3): бак полон,
    // либо уровень перестал расти (клапан отсечки колонки)
    if (after >= 0.999 || after < before + 1e-9)
    {
        state = State::Completed;

        Journal::instance()->info(QString(
            "[SERVICE] Refueling COMPLETED (%1): delivered %2")
            .arg(getResourceString())
            .arg(delivered, 0, 'f', 1));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ServiceSystem::currentLevel(DieselEngineSystem& diesel,
                                   SandSystem& sand) const
{
    switch (resource)
    {
    case Resource::Fuel:
        return diesel.getFuelLevel();

    case Resource::Oil:
        return diesel.getOilLevel();

    case Resource::Coolant:
        return diesel.getCoolantLevel();

    case Resource::Sand:
        return (sand.getCapacity() > 0.0)
                ? sand.getAmount() / sand.getCapacity()
                : 1.0;
    }

    return 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ServiceSystem::resourceAvailable(Resource resource,
                                      DieselEngineSystem& diesel,
                                      SandSystem& sand) const
{
    switch (resource)
    {
    case Resource::Fuel:
    case Resource::Oil:
    case Resource::Coolant:
        // Топливо/масло/ОЖ есть только у ПЕ с дизелем
        return diesel.isConfigured();

    case Resource::Sand:
        return sand.isEnabled();
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ServiceSystem::deliverPortion(double portion,
                                   DieselEngineSystem& diesel,
                                   SandSystem& sand) const
{
    if (portion <= 0.0)
        return;

    // Штатные API систем-приёмников: малые порции, ёмкость не превышается
    switch (resource)
    {
    case Resource::Fuel:
        diesel.refuel(portion, 0.0);
        break;

    case Resource::Oil:
        diesel.refuel(0.0, portion);
        break;

    case Resource::Coolant:
        diesel.topUpCoolant(portion);
        break;

    case Resource::Sand:
        sand.refill(portion);
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ServiceSystem::State ServiceSystem::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ServiceSystem::getStateString() const
{
    switch (state)
    {
    case State::Disconnected:   return "disconnected";
    case State::Connected:      return "connected";
    case State::Refueling:      return "refueling";
    case State::Completed:      return "completed";
    case State::Aborted:        return "aborted";
    }

    return "unknown";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ServiceSystem::Resource ServiceSystem::getResource() const
{
    return resource;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ServiceSystem::getResourceString() const
{
    switch (resource)
    {
    case Resource::Fuel:    return "fuel";
    case Resource::Oil:     return "oil";
    case Resource::Coolant: return "coolant";
    case Resource::Sand:    return "sand";
    }

    return "unknown";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ServiceSystem::getProgress() const
{
    if (state == State::Completed)
        return 1.0;

    if (state == State::Disconnected || initial_level < 0.0)
        return 0.0;

    // Прогресс: доля пути от уровня на момент подключения до полного
    const double rest = 1.0 - initial_level;

    if (rest <= 1e-6)
        return 1.0;

    return std::min(std::max((last_level - initial_level) / rest, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ServiceSystem::isMovementBlocked() const
{
    return state == State::Connected || state == State::Refueling;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ServiceSystem::getDelivered() const
{
    return delivered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ServiceSystem::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ServiceSystem::getLastError() const
{
    return last_error;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ServiceSystem::getDebugMsg() const
{
    return QString("Service: %1 (%2), progress %3%, delivered %4")
            .arg(getStateString())
            .arg(getResourceString())
            .arg(getProgress() * 100.0, 0, 'f', 0)
            .arg(delivered, 0, 'f', 1);
}
