#include    <MouseControlHandler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MouseControlHandler::MouseControlHandler(vsg::ref_ptr<vsg::Camera> camera,
                                         vsg::ref_ptr<vsg::Keyboard> keyboard,
                                         VehiclesHandler *vehicles_handler)
    : _camera(camera)
    , _keyboard(keyboard)
    , _vehicles_handler(vehicles_handler)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::FrameEvent &frameEvent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::MoveEvent &moveEvent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::ButtonPressEvent &buttonPress)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::ButtonReleaseEvent &buttonRelease)
{

}
