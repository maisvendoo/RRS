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

    if (buttonPress.button == 1 || buttonPress.button == 3)
    {
        io_control_input_t input;
        IOController *io_controller = nullptr;

        if (pickControl(static_cast<int>(buttonPress.x),
                        static_cast<int>(buttonPress.y),
                        io_controller,
                        input))
        {
            if (input.id == 0)
            {
                return;
            }

            if (io_controller == nullptr)
            {
                return;
            }

            io_controller->mouseButtonPress(input, buttonPress.button);

            buttonPress.handled = true;
        }
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

    if (buttonRelease.button == 1 || buttonRelease.button == 3)
    {
        io_control_input_t input;
        IOController *io_controller = nullptr;

        if (pickControl(static_cast<int>(buttonRelease.x),
                        static_cast<int>(buttonRelease.y),
                        io_controller,
                        input))
        {
            if (input.id == 0)
            {
                return;
            }

            if (io_controller == nullptr)
            {
                return;
            }

            io_controller->mouseButtonRelease(input, buttonRelease.button);

            buttonRelease.handled = true;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MouseControlHandler::pickControl(int x,
                                      int y,
                                      IOController* &io_ctrl,
                                      io_control_input_t &input)
{
    VehicleExterior *vehicle = _vehicles_handler->getCurrentVehicle();

    if (vehicle == nullptr || vehicle->io_controls.empty() || (_camera == nullptr))
    {
        return false;
    }

    static const int offsets[5][2] = {
        {0, 0}, {9, 0}, {-9, 0}, {0, -9}
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

                for (auto *io_controller : vehicle->io_controls)
                {
                    if (io_controller == nullptr)
                    {
                        continue;
                    }

                    io_controller->findControl(node_name, input);

                    if (input.id == 0)
                    {
                        continue;
                    }

                    // Сохраняем актуальный контроллер ввода
                    io_ctrl = io_controller;

                    if (_last_hit_object != node_name)
                    {
                        _last_hit_object = node_name;
                        LOG_INFO("Clicked: %s state: %3.1f", node_name.c_str(), input.value);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}
