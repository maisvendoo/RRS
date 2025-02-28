#include    <VehiclesUpdateHandler.h>
#include    <vsg/ui/ApplicationEvent.h>

VehiclesUpdateHandler::VehiclesUpdateHandler(VehiclesHandler *veh_handler)
    : vehicles_handler(veh_handler)
//    , _keyboard(vsg::Keyboard::create())
{
}

void VehiclesUpdateHandler::apply(vsg::FrameEvent &event)
{
    if (event.frameStamp->frameCount)
    {
        double t = event.frameStamp->simulationTime;
        double dt = t - prev_time;
        prev_time = t;
        vehicles_handler->step(t, dt);
    }
}

void VehiclesUpdateHandler::apply(vsg::KeyPressEvent& keyPress)
{
    //if (keyPress.handled) return;
    //if (_keyboard) keyPress.accept(*_keyboard);

    if (!vehicles_handler->isUpdated()) return;

    if (keyPress.keyBase == vsg::KEY_Home)
    {
        vehicles_handler->selectNextTrain();
        return;
    }

    if (keyPress.keyBase == vsg::KEY_End)
    {
        vehicles_handler->selectPrevTrain();
        return;
    }

    if (keyPress.keyBase == vsg::KEY_Page_Up)
    {
        vehicles_handler->selectNextVehicle();
        return;
    }

    if (keyPress.keyBase == vsg::KEY_Page_Down)
    {
        vehicles_handler->selectPrevVehicle();
        return;
    }

    if ((keyPress.keyBase == vsg::KEY_KP_Enter) || (keyPress.keyBase == vsg::KEY_Return))
    {
        vehicles_handler->selectControlVehicle();
        return;
    }
}

void VehiclesUpdateHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
//    if (_keyboard) keyRelease.accept(*_keyboard);
}

void VehiclesUpdateHandler::apply(vsg::FocusInEvent& focusIn)
{
//    if (_keyboard) focusIn.accept(*_keyboard);
}

void VehiclesUpdateHandler::apply(vsg::FocusOutEvent& focusOut)
{
//    if (_keyboard) focusOut.accept(*_keyboard);
}
