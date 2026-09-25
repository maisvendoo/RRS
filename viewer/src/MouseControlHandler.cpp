#include    <MouseControlHandler.h>

#include    <VehiclesHandler.h>
#include    <Logger.h>

#include    <vsg/utils/LineSegmentIntersector.h>

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
    (void) frameEvent;

    if (is_Alt_pressed)
    {
        updateTooltip();
    }
    else if (getControlTooltip().is_active)
    {
        getControlTooltip().is_active = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::MoveEvent &moveEvent)
{
    _pointer_x = moveEvent.x;
    _pointer_y = moveEvent.y;
    _pointer_valid = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::ButtonPressEvent &buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    ControlHandler *handler = nullptr;
    IOController *io_controller = nullptr;

    // Определяем что мы попали в какой-то орган управления,
    // возвращаем I/O-контроллер его обрабатываниющий и описатель сигнала
    if (pickControl(static_cast<int>(buttonPress.x),
                    static_cast<int>(buttonPress.y),
                    io_controller,
                    handler))
    {
        if (handler == nullptr)
        {
            return;
        }

        // Если сигнал или контроллер невалидны - уходим
        if (handler->id == 0)
        {
            return;
        }

        // Передаем в данные обработчику нажатия кнопки мыши
        handler->processMouseInput(buttonPress.button, true);

        // Помечаем нажатие как обработанное
        buttonPress.handled = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::ButtonReleaseEvent &buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    ControlHandler *handler = nullptr;
    IOController *io_controller = nullptr;

    if (pickControl(static_cast<int>(buttonRelease.x),
                    static_cast<int>(buttonRelease.y),
                    io_controller,
                    handler))
    {
        if (handler == nullptr)
        {
            return;
        }

        // Если сигнал или контроллер невалидны - уходим
        if (handler->id == 0)
        {
            return;
        }

        handler->processMouseInput(buttonRelease.button, false);

        buttonRelease.handled = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::KeyPressEvent &keyPress)
{
    if (_keyboard)
    {
        keyPress.accept(*_keyboard);
    }

    if (keyPress.keyModified == vsg::KEY_Alt_L || keyPress.keyModified == vsg::KEY_Alt_R)
    {
        is_Alt_pressed = true;
    }

    //LOG_INFO("Alt state: %d", is_Alt_pressed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::apply(vsg::KeyReleaseEvent &keyRelease)
{
    if (_keyboard)
    {
        keyRelease.accept(*_keyboard);
    }

    if (keyRelease.keyModified == vsg::KEY_Alt_L || keyRelease.keyModified == vsg::KEY_Alt_R)
    {
        is_Alt_pressed = false;
    }

    //LOG_INFO("Alt state: %d", is_Alt_pressed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MouseControlHandler::pickControl(int x,
                                      int y,
                                      IOController* &io_ctrl,
                                      ControlHandler *&handler)
{
    VehicleExterior *vehicle = _vehicles_handler->getCurrentVehicle();

    if (vehicle == nullptr || vehicle->io_controller == nullptr || (_camera == nullptr))
    {
        return false;
    }

    static const int offsets[5][2] = {
        {0, 0}, {9, 0}, {-9, 0}, {0, -9}, {0, 9}
    };

    vsg::ref_ptr<vsg::LineSegmentIntersector> intersector;

    for (auto &off : offsets)
    {
        intersector = vsg::LineSegmentIntersector::create(*_camera, x + off[0], y + off[1]);
        vehicle->transform->accept(*intersector);

        for (const auto &hit : intersector->intersections)
        {
            for (auto node_it = hit->nodePath.rbegin(); node_it != hit->nodePath.rend(); ++node_it)
            {
                std::string node_name = "";
                (*node_it)->getValue("name", node_name);

                if (node_name.empty())
                {
                    (*node_it)->getValue("Name", node_name);
                }

                if (node_name.empty())
                {
                    continue;
                }

                if (vehicle->io_controller == nullptr)
                {
                    continue;
                }

                if (!vehicle->io_controller->findControl(node_name, handler))
                {
                    continue;
                }

                if (handler->id == 0)
                {
                    continue;
                }

                // Сохраняем актуальный контроллер ввода
                io_ctrl = vehicle->io_controller;

                if (_last_hit_object != node_name)
                {
                    _last_hit_object = node_name;
                    //LOG_INFO("Clicked: %s state: %3.1f", node_name.c_str(), input.value);
                }

                return true;

            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MouseControlHandler::updateTooltip()
{
    ControlTooltip &tip = getControlTooltip();

    IOController *io_controller = nullptr;
    ControlHandler *handler = nullptr;

    tip.is_active = pickControl(static_cast<int>(_pointer_x), static_cast<int>(_pointer_y), io_controller, handler);

    if (!tip.is_active)
    {
        return;
    }

    if (io_controller == nullptr || handler == nullptr)
    {
        return;
    }

    tip.x = _pointer_x;
    tip.y = _pointer_y;

    tip.title = handler->name;
    tip.description = handler->description;
    tip.usage = handler->usage;
    tip.hot_keys = handler->hot_keys;

    //QString state = QString("Статус: %1").arg(io_controller->getSignalValueByID(input.id, input.cabine_idx), 3, 'f', 1);

    //tip.state = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControlTooltip &getControlTooltip()
{
    static ControlTooltip tooltip;
    return tooltip;
}
