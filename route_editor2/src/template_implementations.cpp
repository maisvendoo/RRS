#include "editor/Action.h"
#include "editor/Keyboard.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>

#include <array>

template class std::array<const char*, TOTAL_ACTIONS>;
template class vsg::ref_ptr<Keyboard>;
template class vsg::ref_ptr<vsg::LookAt>;
template class vsg::ref_ptr<vsg::Orthographic>;
template class vsg::ref_ptr<vsg::Perspective>;
