#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <vsg/core/ref_ptr.h>

namespace vsg
{

class Options;

}

vsg::ref_ptr<vsg::Options> create_default_vsg_options();

#endif // GRAPHICS_COMMON_H
