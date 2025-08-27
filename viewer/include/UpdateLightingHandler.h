#ifndef UPDATE_LIGHTING_HANDLER_H
#define UPDATE_LIGHTING_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

class Sun;
class VehiclesHandler;

namespace vsg
{

class FrameEvent;

};

class UpdateLightingHandler : public vsg::Inherit<vsg::Visitor, UpdateLightingHandler>
{
public:
    UpdateLightingHandler(vsg::ref_ptr<Sun> sun, VehiclesHandler* vehicles_handler);

    void apply(vsg::FrameEvent& frame) override;

private:
    vsg::ref_ptr<Sun> sun;
    VehiclesHandler* vehicles_handler = nullptr;
    double prev_time = 0.0;
};

#endif // UPDATE_LIGHTING_HANDLER_H
