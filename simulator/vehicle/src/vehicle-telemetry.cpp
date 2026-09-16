//------------------------------------------------------------------------------
//
//      Реестр источников телеметрии подвижного состава
//
//------------------------------------------------------------------------------

#include    "vehicle-telemetry.h"

#include    "vehicle.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleTelemetry& VehicleTelemetry::instance()
{
    static VehicleTelemetry reg;
    return reg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleTelemetry::bind(Vehicle* vehicle, Sources sources)
{
    registry[vehicle] = std::move(sources);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleTelemetry::unbind(Vehicle* vehicle)
{
    registry.erase(vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const VehicleTelemetry::Sources* VehicleTelemetry::sourcesOf(const Vehicle* vehicle) const
{
    auto it = registry.find(vehicle);
    return (it != registry.end()) ? &it->second : nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleTelemetry::equalizingReservoirPressure(const Vehicle* vehicle) const
{
    const Sources* s = sourcesOf(vehicle);
    return (s != nullptr && s->equalizing_reservoir) ? s->equalizing_reservoir() : -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleTelemetry::brakePipePressure(const Vehicle* vehicle) const
{
    const Sources* s = sourcesOf(vehicle);
    return (s != nullptr && s->brake_pipe) ? s->brake_pipe() : -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleTelemetry::brakeCylinderPressure(const Vehicle* vehicle) const
{
    const Sources* s = sourcesOf(vehicle);
    return (s != nullptr && s->brake_cylinder) ? s->brake_cylinder() : -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleTelemetry::mainReservoirPressure(const Vehicle* vehicle) const
{
    const Sources* s = sourcesOf(vehicle);
    return (s != nullptr && s->main_reservoir) ? s->main_reservoir() : -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleTelemetry::tractionCurrent(const Vehicle* vehicle) const
{
    const Sources* s = sourcesOf(vehicle);
    return (s != nullptr && s->traction_current) ? s->traction_current() : -1.0;
}
