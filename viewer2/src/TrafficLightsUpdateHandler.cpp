#include    <TrafficLightsUpdateHandler.h>

TrafficLightsUpdateHandler::TrafficLightsUpdateHandler(TrafficLightsHandler *tl_handler)
{
    traffic_light_handler = tl_handler;
}

void TrafficLightsUpdateHandler::apply(vsg::FrameEvent &event)
{
    if (traffic_light_handler == nullptr)
    {
        return;
    }

    traffic_light_handler->update();
}
