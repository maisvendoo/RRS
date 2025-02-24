#ifndef     TRAFFIC_LIHGT_UPDATE_HANDLER_H
#define     TRAFFIC_LIHGT_UPDATE_HANDLER_H

#include    <vsg/core/Visitor.h>
#include    <TrafficLightsHandler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsUpdateHandler : public vsg::Inherit<vsg::Visitor, TrafficLightsUpdateHandler>
{
public:

    TrafficLightsUpdateHandler(TrafficLightsHandler *tl_handler);

    void apply(vsg::FrameEvent &event) override;

private:

    TrafficLightsHandler *traffic_light_handler = nullptr;
};

#endif // TRAFFIC_LIHGT_UPDATE_HANDLER_H
