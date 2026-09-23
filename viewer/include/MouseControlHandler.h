#ifndef     MOUSE_HANDLER_H
#define     MOUSE_HANDLER_H

#include    <vsg/ui/ApplicationEvent.h>
#include    <vsg/ui/PointerEvent.h>
#include    <vsg/ui/KeyEvent.h>
#include    <vsg/ui/Keyboard.h>
#include    <vsg/app/Camera.h>
#include    <vsg/core/Inherit.h>
#include    <vsg/core/ref_ptr.h>

#include    <io-controller.h>

class   VehiclesHandler;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct ControlTooltip
{
    bool is_active = false;
    float x = 0.0f;
    float y = 0.0f;
    QString title = "";
    QString description = "";
    QString state = "";
    QString hot_keys = "";
    QString usage = "";
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControlTooltip &getControlTooltip();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MouseControlHandler : public vsg::Inherit<vsg::Visitor, MouseControlHandler>
{
public:

    MouseControlHandler(vsg::ref_ptr<vsg::Camera> camera,
                        vsg::ref_ptr<vsg::Keyboard> keyboard,
                        VehiclesHandler *vehicles_handler);

    ~MouseControlHandler() = default;

    void apply(vsg::FrameEvent &frameEvent) override;

    void apply(vsg::MoveEvent &moveEvent) override;

    void apply(vsg::ButtonPressEvent &buttonPress) override;

    void apply(vsg::ButtonReleaseEvent &buttonRelease) override;

    void apply(vsg::KeyPressEvent &keyPress) override;

    void apply(vsg::KeyReleaseEvent &keyRelease) override;

private:

    vsg::ref_ptr<vsg::Camera> _camera;
    vsg::ref_ptr<vsg::Keyboard> _keyboard;
    VehiclesHandler *_vehicles_handler = nullptr;

    float _pointer_x = -1.0f;
    float _pointer_y = -1.0f;
    bool _pointer_valid = false;

    bool is_Alt_pressed = false;

    std::string _last_hit_object = "";

    bool pickControl(int x, int y,
                     IOController *&io_ctrl,
                     io_control_input_t& input);

    void updateTooltip();
};

#endif
