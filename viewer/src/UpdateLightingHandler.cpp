#include "UpdateLightingHandler.h"

#include "datetime.h"
#include "Sun.h"
#include "VehiclesHandler.h"

#include <vsg/ui/ApplicationEvent.h>

UpdateLightingHandler::UpdateLightingHandler(vsg::ref_ptr<Sun> sun, VehiclesHandler* vehicles_handler)
    : sun(sun)
    , vehicles_handler(vehicles_handler)
{
}

void UpdateLightingHandler::apply(vsg::FrameEvent& frame)
{
    const double t = frame.frameStamp->simulationTime;
    const double dt = t - prev_time;
    prev_time = t;

    if (dt < 1e-5)
    {
        return;
    }

    const simulator_time_t* const sim_time = vehicles_handler->getDateTime();
    if (sim_time == nullptr)
    {
        return;
    }

    // sun->calculate_direction(
        // sim_time->date.year(), sim_time->date.month(), sim_time->date.day(),
        // sim_time->time.hour(), sim_time->time.minute(), sim_time->time.sec(),
        // 3.0,
        // 0.0, 0.0
    // );
}
