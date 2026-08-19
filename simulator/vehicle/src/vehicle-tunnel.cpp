//------------------------------------------------------------------------------
//
//      Tunnel aerodynamics (эффект "воздушного поршня" в тоннеле)
//
//------------------------------------------------------------------------------

#include    "vehicle-tunnel.h"

#include    <CfgReader.h>

#include    <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TunnelAerodynamics::TunnelAerodynamics() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TunnelAerodynamics::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "TunnelAero";

    cfg.getBool(sec, "Enabled", enabled);
    cfg.getDouble(sec, "TunnelArea", tunnel_area);
    cfg.getDouble(sec, "TrainArea", train_area);
    cfg.getDouble(sec, "PistonCoeff", piston_coeff);
    cfg.getDouble(sec, "AirDensity", air_density);

    tunnel_area = std::max(tunnel_area, 1.0);
    train_area = std::max(train_area, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TunnelAerodynamics::setZones(const std::vector<std::pair<double, double>>& zones_)
{
    zones.clear();

    for (const auto& z : zones_)
    {
        // Зона нулевой/отрицательной длины отбрасывается
        if (z.second > z.first)
            zones.push_back(z);
    }

    std::sort(zones.begin(), zones.end(),
              [](const Zone& a, const Zone& b)
    {
        return a.first < b.first;
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TunnelAerodynamics::inTunnel(double coord_) const
{
    for (const Zone& z : zones)
    {
        if (coord_ >= z.first && coord_ <= z.second)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TunnelAerodynamics::setCoordinate(double railway_coord)
{
    coord = railway_coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TunnelAerodynamics::step(double dt, double velocity, double train_length_)
{
    (void) velocity;

    if (!enabled || dt <= 0.0)
        return;

    train_length = std::max(train_length_, 1.0);

    if (zones.empty())
    {
        immersion = 0.0;
        return;
    }

    //--- Погруженность: доля длины ПЕ внутри зон тоннеля.
    // Первая ось заходит за портал - коэффициент начинает расти,
    // последняя ось выходит - падает до нуля (плавный вход/выход)

    const double half = 0.5 * train_length;
    const double head = coord + half;   // передний край ПЕ
    const double tail = coord - half;   // задний край ПЕ

    double inside = 0.0;

    for (const Zone& z : zones)
    {
        // Перекрытие отрезка [tail, head] с зоной [z.first, z.second]
        const double overlap = std::min(head, z.second) -
                std::max(tail, z.first);

        if (overlap > 0.0)
            inside += overlap;
    }

    immersion = std::min(inside / train_length, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TunnelAerodynamics::getImmersion() const
{
    return enabled ? immersion : 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TunnelAerodynamics::getResistanceForce(double velocity) const
{
    if (!enabled || immersion <= 0.0)
        return 0.0;

    // Формула поршня (ТЗ "43-47", п.3): скоростной напор на мидель ПЕ,
    // усиленный квадратом отношения сечений поезд/тоннель. Чем теснее
    // тоннель, тем сильнее эффект
    const double blockage = std::min(train_area / tunnel_area, 0.9);

    const double v = std::abs(velocity);

    return piston_coeff * 0.5 * air_density * train_area *
            blockage * blockage * v * v * immersion;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TunnelAerodynamics::isConfigured() const
{
    return enabled && !zones.empty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString TunnelAerodynamics::getDebugMsg() const
{
    return QString("Tunnel: %1 zones, immersion %2%, force at 30 m/s: %3 kN")
            .arg(zones.size())
            .arg(immersion * 100.0, 0, 'f', 0)
            .arg(getResistanceForce(30.0) / 1000.0, 0, 'f', 1);
}
