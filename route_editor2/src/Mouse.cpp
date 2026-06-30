#include "editor/Mouse.h"

bool Mouse::is_lmb_pressed() const
{
    return lmb_state;
}

bool Mouse::is_mmb_pressed() const
{
    return mmb_state;
}

bool Mouse::is_rmb_pressed() const
{
    return rmb_state;
}

