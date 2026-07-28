#include    <InputRouteHandler.h>

#include    <VehiclesHandler.h>
#include    <Logger.h>

#include    <key-symbols.h>

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
    if (!m_hasFocus || !m_activeIOController)
    {
        keyPress.handled = false;
        return;
    }

    const uint16_t keyBase = keyPress.keyBase;

    if (!isControlKey(keyBase))
    {
        keyPress.handled = false;
        return;
    }

    processKeyForController(keyBase, true);
    keyPress.handled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::apply(vsg::KeyReleaseEvent &keyRelease)
{
    if (!m_hasFocus || !m_activeIOController)
    {
        keyRelease.handled = false;
        return;
    }

    const uint16_t keyBase = keyRelease.keyBase;

    if (!isControlKey(keyBase))
    {
        keyRelease.handled = false;
        return;
    }

    processKeyForController(keyBase, true);
    keyRelease.handled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::apply(vsg::FrameEvent &frame)
{
    if (!m_activeIOController)
    {
        return;
    }

    if (frame.frameStamp->frameCount)
    {
        const double t = frame.frameStamp->simulationTime;
        const double dt = t - _previousTime;
        _previousTime = t;

        updateController(static_cast<float>(dt));
    }

    frame.handled = false;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool InputRouteHandler::isControlKey(uint16_t key)
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::processKeyForController(uint16_t keyBase, bool pressed)
{
    if (!m_activeIOController)
    {
        return;
    }

    if (!KeySymbolsRRS.count(keyBase))
    {
        LOG_WARN("InputRouteHandler: Unknown key 0x%04X", keyBase);
        return;
    }

    if (pressed)
    {
        if(m_pressedKeys.insert(keyBase).second)
        {
            m_activeIOController->setPressedKey(keyBase);
            LOG_DEBUG("InputRouteHandler: Control key pressed 0x%04X", keyBase);
        }
    }
    else
    {
        if (m_pressedKeys.erase(keyBase))
        {
            m_activeIOController->setReleasedKey(keyBase);
            LOG_DEBUG("InputRouteHandler: Control key released 0x%04X", keyBase);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::updateController(float dt)
{
    if (m_activeIOController)
    {
        m_activeIOController->step(0.0f, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void InputRouteHandler::resetControllerState()
{
    if (!m_activeIOController)
    {
        return;
    }

    for (auto key : m_pressedKeys)
    {
        m_activeIOController->setReleasedKey(key);
    }

    m_pressedKeys.clear();
}
