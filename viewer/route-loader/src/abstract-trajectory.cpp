#include    "abstract-trajectory.h"

Trajectory::Trajectory()
{

}

MotionPath *Trajectory::getMotionPath()
{
    return routePath.get();
}
