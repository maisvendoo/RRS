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
    (void)frame;

    const simulator_time_t* const sim_time = vehicles_handler->getDateTime();
    if (sim_time == nullptr)
    {
        return;
    }

    sun->update(
        sim_time->date.year(), sim_time->date.month(), sim_time->date.day(),
        sim_time->time.hour(), sim_time->time.minute(), sim_time->time.sec() + sim_time->time.msec() / 1000.0,
        3.0
    );
}
