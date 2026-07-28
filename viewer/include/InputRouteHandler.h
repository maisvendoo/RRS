#ifndef     INPUT_ROUTE_HANDLER_H
#define     INPUT_ROUTE_HANDLER_H

#include    <vsg/ui/ApplicationEvent.h>
#include    <vsg/ui/KeyEvent.h>
#include    <vsg/ui/PointerEvent.h>
#include    <vsg/core/Inherit.h>
#include    <vsg/core/ref_ptr.h>
#include    <vsg/ui/Keyboard.h>
#include    <set>

#include    "io-controller.h"

class VehiclesHandler;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class InputRouteHandler : public vsg::Inherit<vsg::Visitor, InputRouteHandler>
{
public:

    InputRouteHandler();

    ~InputRouteHandler() = default;

    void apply(vsg::KeyPressEvent& keyPress) override;

    void apply(vsg::KeyReleaseEvent& keyRelease) override;

    void apply(vsg::FrameEvent &frame) override;

    void setKeyboard(vsg::ref_ptr<vsg::Keyboard> keyboard);

    void setActiveController(IOController *io_controller);

    void clearActiveController();

private:

    vsg::ref_ptr<vsg::Keyboard> m_keyboard;

    IOController *m_activeIOController = nullptr;

    VehiclesHandler *m_vehiclesHandler = nullptr;

    std::set<uint16_t> m_pressedKeys;

    bool m_hasFocus = true;

    double _previousTime = 0.0;

    bool isControlKey(uint16_t key);

    void processKeyForController(uint16_t keyBase, bool pressed);

    void updateController(float dt);

    void resetControllerState();
};

#endif
