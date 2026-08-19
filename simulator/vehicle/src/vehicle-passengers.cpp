//------------------------------------------------------------------------------
//
//      Passenger system (посадка/высадка пассажиров)
//
//------------------------------------------------------------------------------

#include    "vehicle-passengers.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassengerSystem::PassengerSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassengerSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "PassengerCar";

    int ivalue = 0;
    double value = 0.0;
    bool found = false;

    if (cfg.getInt(sec, "Capacity", ivalue))
    {
        capacity = std::max(ivalue, 1);
        found = true;
    }
    if (cfg.getInt(sec, "CurrentPassengers", ivalue))
    {
        passengers = std::max(ivalue, 0);
        found = true;
    }
    if (cfg.getInt(sec, "Doors", ivalue))
        doors = std::max(ivalue, 1);
    if (cfg.getDouble(sec, "DoorWidth", value))
        door_width = std::min(std::max(value, 0.5), 2.0);
    if (cfg.getDouble(sec, "PassRate", value))
        pass_rate = std::min(std::max(value, 0.1), 5.0);
    cfg.getBool(sec, "HasStairs", has_stairs);

    passengers = std::min(passengers, capacity);
    configured = found;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::isConfigured() const
{
    return configured;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::hasStairs() const
{
    return has_stairs;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassengerSystem::setDoorsOpen(bool open)
{
    if (doors_open && !open)
    {
        // Закрытие дверей прекращает потоки (п.28)
        boarding_queue = 0;
        alighting_left = 0;
    }

    doors_open = open;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::isDoorsOpen() const
{
    return doors_open;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassengerSystem::beginBoarding(int waiting)
{
    if (!doors_open || waiting <= 0)
        return;

    boarding_queue += waiting;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassengerSystem::beginAlighting(int exiting)
{
    if (!doors_open || exiting <= 0)
        return;

    alighting_left += std::min(exiting, passengers);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::isBoardingInProgress() const
{
    return boarding_queue > 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::isAlightingInProgress() const
{
    return alighting_left > 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassengerSystem::step(double dt)
{
    if (!doors_open)
        return;

    // Скорость через двери: узкие двери и толпа замедляют (п.25)
    double rate = pass_rate * static_cast<double>(doors) *
            (door_width / 0.9);

    // Очередь замедляет поток при большом скоплении
    if (boarding_queue + alighting_left > 30)
        rate *= 0.7;

    //--- Высадка идёт первой (освобождает места, п.24) ---
    if (alighting_left > 0)
    {
        alighting_accum += std::min(static_cast<double>(alighting_left),
                                    rate * dt);

        const int out = static_cast<int>(alighting_accum);

        alighting_accum -= out;
        alighting_left -= out;
        passengers = std::max(0, passengers - out);
    }

    //--- Посадка ---
    if (boarding_queue > 0)
    {
        boarding_accum += std::min(static_cast<double>(boarding_queue),
                                   rate * dt);

        int in = static_cast<int>(boarding_accum);
        boarding_accum -= in;

        // Вместимость ограничивает (п.26)
        const int free = capacity - passengers;
        in = std::min(in, free);

        passengers += in;
        boarding_queue -= in;

        if (boarding_queue > 0 && free <= 0)
        {
            // Не поместились: передаются другим вагонам (п.26)
            overflow += boarding_queue;
            boarding_queue = 0;

            Journal::instance()->info(QString(
                "[PASSENGER] Car full: %1 passengers redirected")
                .arg(overflow));
        }
    }

    // Переполнение забирается распределением (внешней системой)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int PassengerSystem::getPassengerCount() const
{
    return passengers;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int PassengerSystem::getCapacity() const
{
    return capacity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PassengerSystem::hasFreeSpace() const
{
    return passengers < capacity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PassengerSystem::getPassengerMass() const
{
    // 90 кг: пассажир с багажом (п.32)
    return static_cast<double>(passengers) * 90.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PassengerSystem::getBoardingRate() const
{
    double rate = pass_rate * static_cast<double>(doors) *
            (door_width / 0.9);

    if (boarding_queue + alighting_left > 30)
        rate *= 0.7;

    return rate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int PassengerSystem::getOverflowQueue() const
{
    return overflow;
}
