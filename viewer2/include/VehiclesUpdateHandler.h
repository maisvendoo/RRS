#ifndef     VEHICLES_UPDATE_HANDLER_H
#define     VEHICLES_UPDATE_HANDLER_H

#include    <vsg/core/Inherit.h>
#include    <vsg/core/Visitor.h>
#include    <vsg/ui/Keyboard.h>

#include    "VehiclesHandler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehiclesUpdateHandler : public vsg::Inherit<vsg::Visitor, VehiclesUpdateHandler>
{
public:

    VehiclesUpdateHandler(VehiclesHandler *veh_handler);

    void apply(vsg::FrameEvent &event) override;
    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;
    void apply(vsg::FocusInEvent& focusIn) override;
    void apply(vsg::FocusOutEvent& focusOut) override;

private:

    VehiclesHandler *vehicles_handler = nullptr;

//    vsg::ref_ptr<vsg::Keyboard> _keyboard = nullptr;

    double prev_time = 0.0;
};

#endif // VEHICLES_UPDATE_HANDLER_H
