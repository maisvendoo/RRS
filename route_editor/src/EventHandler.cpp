#include "EventHandler.h"

#include "states/SelectRoute.h"
#include "states/State.h"

#include <memory>

EventHandler::EventHandler()
    : select_route(std::make_unique<SelectRoute>())
    // , state(&select_route)
{

}

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    (*state)->handle_key_press(keyPress);
}
