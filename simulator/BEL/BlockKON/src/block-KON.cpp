#include    "block-KON.h"
#include    <QDebug>
#include    "CfgReader.h"

constexpr double SPEED_START_MOVEMENT = 2.0;

BlockKON::BlockKON(QObject *parent) : AbstractBlockKON(parent)
{
    connect(control_timer, &Timer::process, this, &BlockKON::onControlTimer);
}

void BlockKON::step(double t, double dt)
{
    control_timer->step(t, dt);

    if(isEPKShutdown() && isMove())
        startControlTimer();

    if(isOnEPK() || !isMove())
    {
        stopControlTimer();
        valve_electrical_supply = false;
    }
}

void BlockKON::setKeyEPK(bool key_epk)
{
    old_key_epk = this->key_epk;
    this->key_epk = key_epk;
}

void BlockKON::setVelocityKmh(double v_kmh)
{
    this->v_kmh = v_kmh;
}

void BlockKON::setBrakeCylinderPressure(double pBC)
{
    this->pBC = pBC;
}

bool BlockKON::getValveElectricalSupply() const
{
    return valve_electrical_supply;
}

void BlockKON::load_config(CfgReader &cfg)
{
    QString sec_name = "Device";

    if(!cfg.getDouble(sec_name, "PermissiblePressureBC", block_kon_cfg.permissible_pressure_BC))
    {
        constexpr double DEFAULT_PERMISSIBLE_PRESSURE = 0.068;

        block_kon_cfg.permissible_pressure_BC = DEFAULT_PERMISSIBLE_PRESSURE;

        qWarning() << "Warning: Permissible pressure in brake cylinders configuration not found."
                   << "Default permissible pressure in brake cylinders:" << DEFAULT_PERMISSIBLE_PRESSURE;
    }

    if(!cfg.getInt(sec_name, "ControlInterval", block_kon_cfg.control_interval))
    {
        constexpr int DEFAULT_CONTROL_INTERVAL = 11;

        block_kon_cfg.control_interval = DEFAULT_CONTROL_INTERVAL;

        qWarning() << "Warning: Control timer interval configuration not found."
                   << "Default control timer interval:" << DEFAULT_CONTROL_INTERVAL;
    }

    control_timer->setTimeout(block_kon_cfg.control_interval);
}

bool BlockKON::isMove()
{
    return v_kmh > SPEED_START_MOVEMENT ? true : false;
}

bool BlockKON::isEPKShutdown()
{
    return old_key_epk && !key_epk ? true : false;
}

bool BlockKON::isOnEPK()
{
    return !old_key_epk && key_epk ? true : false;
}

void BlockKON::startControlTimer() const
{
    if(!control_timer->isStarted())
        control_timer->start();
}

void BlockKON::stopControlTimer() const
{
    if(control_timer->isStarted())
        control_timer->stop();
}

void BlockKON::onControlTimer()
{
    if(pBC < block_kon_cfg.permissible_pressure_BC)
        valve_electrical_supply = true;
}

GET_PLUGIN_BLOCK_KON(BlockKON)
