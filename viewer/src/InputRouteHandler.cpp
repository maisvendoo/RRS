#include    <InputRouteHandler.h>

#include    <VehiclesHandler.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
InputRouteHandler::InputRouteHandler()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::apply(vsg::KeyPressEvent &keyPress)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::apply(vsg::KeyReleaseEvent &keyRelease)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::setKeyboard(vsg::ref_ptr<vsg::Keyboard> keyboard)
{
    m_keyboard = keyboard;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::setActiveController(IOController *io_controller)
{
    if (m_activeIOController == io_controller)
    {
        return;
    }

    if (m_activeIOController)
    {
        // Сбрасываем состояние старого контроллера
    }

    m_activeIOController = io_controller;
    m_pressedKeys.clear();

    if (io_controller)
    {
        LOG_INFO("InputRouteHandler: IOController activated");
    }
    else
    {
        LOG_INFO("InputRouteHandler: IOController deactivated");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::clearActiveController()
{
    setActiveController(nullptr);
}
