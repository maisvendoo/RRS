//------------------------------------------------------------------------------
//
//      Coupling interaction (интерактивная сцепка СА-3, рукава, краны)
//
//------------------------------------------------------------------------------

#include    "vehicle-coupling-interaction.h"

#include    "vehicle-damage.h"

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
void CouplingInteraction::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    cfg.getDouble("CouplingInteraction", "TearSpeed", tear_speed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::connectHose(End end)
{
    EndState& state = ends[static_cast<int>(end)];

    if (state.hose_torn)
        return false;

    // Оборванный/соединённый рукав нельзя соединить
    if (state.hose == HoseState::Connected)
        return false;

    // Рукав с кронштейна или болтающийся можно соединить с соседним
    state.hose = HoseState::Connected;

    Journal::instance()->info(QString(
        "[COUPLING] Hose %1 connected")
        .arg(end == Fwd ? "fwd" : "bwd"));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::hangHose(End end)
{
    EndState& state = ends[static_cast<int>(end)];

    // Соединённый рукав нельзя вешать: сначала разъединить
    if (state.hose == HoseState::Connected || state.hose_torn)
        return false;

    state.hose = HoseState::OnHolder;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingInteraction::setEndValve(End end, bool open)
{
    EndState& state = ends[static_cast<int>(end)];

    state.valve_open = open;

    Journal::instance()->info(QString(
        "[COUPLING] End valve %1 %2")
        .arg(end == Fwd ? "fwd" : "bwd")
        .arg(open ? "OPEN" : "CLOSED"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::pullUncoupleLever(End end, double speed)
{
    EndState& state = ends[static_cast<int>(end)];

    // Расцепка на ходу запрещена блокировкой (ТЗ, п.2)
    if (std::abs(speed) > 0.5)
    {
        Journal::instance()->warning(
            "[COUPLING] Uncouple lever blocked: vehicle is moving");
        return false;
    }

    state.lever_pulled = true;

    // Если рукав ещё соединён - расцепка срывает его (ошибка машиниста)
    if (state.hose == HoseState::Connected)
    {
        state.hose = HoseState::Hanging;
        state.hose_torn = true;
        state.valve_open = true;

        Journal::instance()->critical(
            "[COUPLING] Uncoupled WITH HOSE CONNECTED - hose torn, "
            "brake pipe broken!");
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingInteraction::step(double dt, double velocity,
                               VehicleDamageSystem& damage)
{
    (void) dt;

    const double abs_v = std::abs(velocity);

    for (int i = 0; i < 2; ++i)
    {
        EndState& state = ends[i];

        if (state.hose_torn)
            continue;

        // Движение с болтающимся рукавом: обрыв (ТЗ, п.3-5)
        if (state.hose == HoseState::Hanging && abs_v > tear_speed)
        {
            state.hose_torn = true;
            state.valve_open = true;

            Journal::instance()->critical(QString(
                "[COUPLING] Hanging hose %1 TORN at speed - "
                "brake pipe broken!")
                .arg(i == 0 ? "fwd" : "bwd"));

            damage.addDamage(VehicleDamageSystem::Component::Brake, 0.2);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CouplingInteraction::HoseState CouplingInteraction::getHoseState(End end) const
{
    return ends[static_cast<int>(end)].hose;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::isEndValveOpen(End end) const
{
    return ends[static_cast<int>(end)].valve_open;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::isLeverPulled(End end) const
{
    return ends[static_cast<int>(end)].lever_pulled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CouplingInteraction::isHoseTorn(End end) const
{
    return ends[static_cast<int>(end)].hose_torn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CouplingInteraction::getDebugMsg() const
{
    QString msg;

    for (int i = 0; i < 2; ++i)
    {
        const EndState& state = ends[i];

        const char* hoses[] = {"connected", "on holder", "hanging"};

        msg += QString("%1: hose %2%3, valve %4, lever %5\n")
                .arg(i == 0 ? "FWD" : "BWD")
                .arg(hoses[static_cast<int>(state.hose)])
                .arg(state.hose_torn ? " [TORN]" : "")
                .arg(state.valve_open ? "open" : "closed")
                .arg(state.lever_pulled ? "pulled" : "off");
    }

    return msg;
}
