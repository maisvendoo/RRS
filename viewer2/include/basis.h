#ifndef VIEWER_BASIS_H
#define VIEWER_BASIS_H

#include <vsg/maths/vec3.h>

struct basis_t
{
    basis_t();

    vsg::vec3 front;
    vsg::vec3 right;
    vsg::vec3 up;
};

#endif // VIEWER_BASIS_H
