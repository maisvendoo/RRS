#include "track.h"

track_t::track_t()
    : ordinate(0)
    , voltage(0)
    , arrows("")
    , begin_point()
    , end_point()
    , prev_uid(-1)
    , next_uid(-2)
    , length(0.0f)
    , orth()
    , attitude()
    , right()
    , rail_coord(0.0f)
{
}
