#include "camera-position.h"

camera_position_t::camera_position_t()
    : position(0.0, 0.0, 0.0)
    , attitude(-osg::PI_2, 0.0, 0.0)
    , driver_pos(0.0, 0.0, 1.75)
    , viewer_pos(0.0, 150.0, 0.0)
    , front(0.0, 1.0, 0.0)
    , right(1.0, 0.0, 0.0)
    , up(0.0, 0.0, 1.0)
    , is_orient_bwd(false)
{
}
