#include    <TrafficLightsUpdateHandler.h>
#include    <vsg/ui/ApplicationEvent.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLightsUpdateHandler::TrafficLightsUpdateHandler(TrafficLightsHandler *tl_handler)
{
    traffic_light_handler = tl_handler;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsUpdateHandler::apply(vsg::FrameEvent &event)
{
    if (traffic_light_handler && event.frameStamp->frameCount)
    {
        double t = event.frameStamp->simulationTime;
        double dt = t - prev_time;
        traffic_light_handler->step(static_cast<float>(t), static_cast<float>(dt));

        prev_time = t;
    }
}
