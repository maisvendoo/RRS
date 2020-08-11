#include    "abstract-path.h"

MotionPath::MotionPath()
    : length(0.0f)
{

}

float MotionPath::getLength() const
{
    return length;
}
