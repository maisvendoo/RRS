//------------------------------------------------------------------------------
//
//      Depot power system (деповское питание 380 В и заряд АБ)
//
//------------------------------------------------------------------------------

#include    "vehicle-depot-power.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DepotPowerSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    cfg.getDouble("DepotPower", "SourceVoltage", source_voltage);
    cfg.getDouble("DepotPower", "SourceMaxPower", source_max_power);
    cfg.getDouble("DepotPower", "CableLength", cable_length);

    cfg.getDouble("Battery", "Capacity", battery_capacity);
    cfg.getDouble("Battery", "Charge", battery_charge);
    cfg.getDouble("Battery", "MinVoltage", battery_min_voltage);
    cfg.getDouble("Battery", "MaxVoltage", battery_max_voltage);
    cfg.getDouble("Battery", "MaxChargeCurrent", max_charge_current);
    cfg.getDouble("Battery", "ChargerPower", charger_power);

    battery_charge = std::min(std::max(battery_charge, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::connectCable(double distance)
{
    // Блокировки (ТЗ, п.5): под напряжением подключать нельзя
    if (source_on)
    {
        last_error = "Нельзя подключать кабель: источник под напряжением";
        Journal::instance()->warning("[DEPOT] " + last_error);
        return false;
    }

    if (distance > cable_length)
    {
        last_error = QString("Кабель короток: %1 м из %2 м")
                .arg(distance, 0, 'f', 1)
                .arg(cable_length, 0, 'f', 1);
        Journal::instance()->warning("[DEPOT] " + last_error);
        return false;
    }

    cable_connected = true;
    last_error = "";
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::disconnectCable()
{
    // Отсоединение под напряжением запрещено (ТЗ, п.5): пока источник
    // включён, на розетке ПЕ есть напряжение - ждём выключения
    if (source_on)
    {
        last_error = "Сначала выключите питание источника";
        Journal::instance()->warning("[DEPOT] " + last_error);
        return false;
    }

    if (cable_connected)
    {
        cable_connected = false;
        input_breaker_on = false;
    }

    last_error = "";
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::setSourcePower(bool on)
{
    if (on && !cable_connected)
    {
        last_error = "Нет подключённого кабеля";
        return false;
    }

    source_on = on;

    // Внешнее питание подаётся, если включён вводной аппарат
    external_power = source_on && cable_connected && input_breaker_on;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DepotPowerSystem::setInputBreaker(bool on)
{
    input_breaker_on = on;

    external_power = source_on && cable_connected && input_breaker_on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isInputBreakerOn() const
{
    return input_breaker_on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DepotPowerSystem::setSourceNearby(bool nearby, double cable_len)
{
    source_nearby = nearby;
    nearby_cable_length = (cable_len > 1.0) ? cable_len : 25.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isSourceNearby() const
{
    return source_nearby;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getNearbyCableLength() const
{
    return nearby_cable_length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isCableConnected() const
{
    return cable_connected;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isExternalPower() const
{
    return external_power;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isMovementBlocked() const
{
    // Пока кабель физически подключён - движение запрещено
    // (потребитель обрежет тягу, ТЗ п.14)
    return cable_connected;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isConnectorDamaged() const
{
    return connector_damaged;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getCableLength() const
{
    return cable_length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getBatteryCharge() const
{
    return battery_charge;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getBatteryVoltage() const
{
    // Напряжение АБ от степени заряда + просадка от тока разряда
    const double base = battery_min_voltage +
            (battery_max_voltage - battery_min_voltage) * battery_charge;

    const double sag = 0.02 * std::abs(std::min(battery_current, 0.0));

    return base - sag;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getBatteryCurrent() const
{
    return battery_current;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DepotPowerSystem::getBatteryTemperature() const
{
    return battery_temperature;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DepotPowerSystem::isBatteryDead() const
{
    return battery_charge <= 0.05;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DepotPowerSystem::step(double dt, double aux_load_w,
                            bool engine_running, bool vehicle_moving)
{
    external_power = source_on && cable_connected && input_breaker_on;

    // Движение с подключённым кабелем (ТЗ, п.14): физический обрыв
    // кабеля + повреждение разъёма локомотива (аварийное событие)
    if (cable_connected && vehicle_moving)
    {
        cable_connected = false;
        external_power = false;
        input_breaker_on = false;
        move_violation = true;
        connector_damaged = true;

        Journal::instance()->critical(
            "[DEPOT] Movement with depot cable connected - cable torn, "
            "connector damaged!");
    }
    else if (!vehicle_moving)
    {
        // Факт нарушения гасится при остановке
        move_violation = false;
    }

    // Ток батареи: заряд от внешнего питания или генератора,
    // разряд от вспомогательных потребителей
    double charge_current = 0.0;

    if (engine_running)
    {
        // Заряд от собственного генератора (зарядное устройство)
        charge_current = max_charge_current;
    }
    else if (external_power)
    {
        // Заряд от деповского источника через ЗУ (ограничен ЗУ и сетью)
        const double charger_current = charger_power /
                std::max(0.5 * (battery_min_voltage + battery_max_voltage), 1.0);

        charge_current = std::min(max_charge_current, charger_current);
    }

    // CC-CV профиль заряда: до 80% SoC - полный ток (CC), выше -
    // линейное снижение к 10% максимума к 100% (CV-этап)
    if (battery_charge > 0.8)
    {
        const double cv_factor = std::max(0.1,
                1.0 - 0.9 * (battery_charge - 0.8) / 0.2);

        charge_current *= cv_factor;
    }

    // Предел мощности деповского источника: зарядная мощность не выше
    // source_max_power, при превышении - ограничение тока и событие
    if (external_power && charge_current > 0.0)
    {
        const double max_source_current = source_max_power /
                std::max(getBatteryVoltage(), 1.0);

        if (charge_current > max_source_current)
        {
            charge_current = max_source_current;

            if (!src_limit_reported)
            {
                src_limit_reported = true;
                Journal::instance()->warning(QString(
                    "[DEPOT] Source power limit %1 W: charge current "
                    "limited to %2 A")
                    .arg(source_max_power, 0, 'f', 0)
                    .arg(charge_current, 0, 'f', 1));
            }
        }
        else
        {
            src_limit_reported = false;
        }
    }

    const double load_current = aux_load_w /
            std::max(getBatteryVoltage(), 1.0);

    // Полностью заряженная батарея принимает только поддерживающий ток
    if (battery_charge >= 1.0)
    {
        charge_current = std::min(charge_current, 2.0);
    }

    battery_current = charge_current - load_current;

    // Интегрирование заряда, А*ч
    battery_charge += battery_current * dt / 3600.0 /
            std::max(battery_capacity, 1.0);

    battery_charge = std::min(std::max(battery_charge, 0.0), 1.0);

    // Температура: самонагрев от тока, охлаждение
    const double heat = 1e-5 * battery_current * battery_current;
    battery_temperature += (heat - 0.002 *
            (battery_temperature - 20.0)) * dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString DepotPowerSystem::getLastError() const
{
    return last_error;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString DepotPowerSystem::getDebugMsg() const
{
    return QString("Depot: cable %1, ext.power %2, battery %3% (%4 V, "
                   "%5 A, %6 C)%7")
            .arg(cable_connected ? "connected" : "off")
            .arg(external_power ? "380V" : "no")
            .arg(battery_charge * 100.0, 0, 'f', 0)
            .arg(getBatteryVoltage(), 0, 'f', 1)
            .arg(battery_current, 0, 'f', 1)
            .arg(battery_temperature, 0, 'f', 0)
            .arg(move_violation ? " [CABLE TORN]"
                                : (connector_damaged ? " [CONNECTOR DMG]" : ""));
}
