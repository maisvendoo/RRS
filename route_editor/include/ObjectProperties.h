#ifndef OBJECT_PROPERTIES_H
#define OBJECT_PROPERTIES_H

#include "ObjectState.h"

#include <vsg/maths/vec3.h>

#include <string>

struct ObjectProperties
{
    ObjectState state = ObjectState::INITIAL;
    std::string name;
    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
};

#endif // OBJECT_PROPERTIES_H
