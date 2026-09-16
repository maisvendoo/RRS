#ifndef     CAB_MOUSE_HANDLER_H
#define     CAB_MOUSE_HANDLER_H

#include    <vsg/ui/ApplicationEvent.h>
#include    <vsg/ui/PointerEvent.h>
#include    <vsg/ui/KeyEvent.h>
#include    <vsg/ui/Keyboard.h>
#include    <vsg/core/Inherit.h>
#include    <vsg/core/ref_ptr.h>

#include    <string>

#include    "io-controller.h"

class VehiclesHandler;

//------------------------------------------------------------------------------
// Состояние подсказки кабины для отрисовки (заполняет CabMouseHandler,
// читает MyGui; один поток UI)
//------------------------------------------------------------------------------
struct CabTooltipState
{
    bool active = false;
    float x = 0.0f;
    float y = 0.0f;
    std::string title;
    std::string state_text;
    std::string action_text;
};

CabTooltipState& cabTooltip();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Взаимодействие мышью с органами кабины поверх архитектуры IOController
/// (бранч v1.10.0-devel-iocontroller): пикинг меша по ObjectName из
/// конфига [Control], клик уходит командой управления через
/// IOController::mouseClick(), подсказка - по Alt+наведению
class CabMouseHandler : public vsg::Inherit<vsg::Visitor, CabMouseHandler>
{
public:

    CabMouseHandler(vsg::ref_ptr<vsg::Camera> camera,
                    vsg::ref_ptr<vsg::Keyboard> keyboard,
                    VehiclesHandler* vehicles_handler);

    ~CabMouseHandler() = default;

    void apply(vsg::ButtonPressEvent& buttonPress) override;

    void apply(vsg::MoveEvent& moveEvent) override;

    void apply(vsg::FrameEvent& frame) override;

private:

    /// Alt удерживается (режим подсказок)
    bool altHeld() const;

    /// Поиск органа под точкой экрана: веер лучей камера->курсор,
    /// имя ноды меша сопоставляется с ObjectName конфигов [Control]
    /// текущей ПЕ (точное совпадение или суффикс)
    bool pickControl(int x, int y,
                     IOController*& io_ctrl,
                     io_control_input_t& input);

    /// Заполнение подсказки (Alt+наведение)
    void updateTooltip();

    vsg::ref_ptr<vsg::Camera> _camera;
    vsg::ref_ptr<vsg::Keyboard> _keyboard;
    VehiclesHandler* _vehicles_handler = nullptr;

    float _pointer_x = -1.0f;
    float _pointer_y = -1.0f;
    bool _pointer_valid = false;

    /// Последний найденный орган (для логгирования повторов)
    std::string _last_hit_object = "";
};

#endif
