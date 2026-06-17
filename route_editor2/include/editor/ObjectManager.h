#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <vector>

namespace vsg
{

class PagedLOD;

}

class ObjectManager
{
public:
    ObjectManager();

    ~ObjectManager();

private:
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
};

#endif // EDITOR_OBJECT_MANAGER_H
