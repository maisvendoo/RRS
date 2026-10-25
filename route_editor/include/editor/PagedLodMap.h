#ifndef PAGED_LOD_MAP_H
#define PAGED_LOD_MAP_H

#include <vsg/core/ref_ptr.h>

#include <map>
#include <string>

namespace vsg
{

class PagedLOD;

}

using PagedLodMap = std::map<std::string, vsg::ref_ptr<vsg::PagedLOD>>;

#endif // PAGED_LOD_MAP_H
