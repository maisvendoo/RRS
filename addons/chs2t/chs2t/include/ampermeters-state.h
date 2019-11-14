#ifndef AMPERMETERS_STATE_H
#define AMPERMETERS_STATE_H

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct ampermeters_state_t
{
    bool    is12on;
    bool    is34on;
    bool    is56on;

    ampermeters_state_t()
        : is12on(false)
        , is34on(false)
        , is56on(false)
    {

    }
};

#endif // AMPERMETERS_STATE_H
