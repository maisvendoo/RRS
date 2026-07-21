#include "states/RouteNotLoadedState.h"

RouteNotLoadedState::RouteNotLoadedState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

RouteNotLoadedState::~RouteNotLoadedState() = default;
