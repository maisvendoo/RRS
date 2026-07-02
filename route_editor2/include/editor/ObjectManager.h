#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <vector>

namespace vsg
{

class MatrixTransform;
class PagedLOD;

}

class ObjectManager
{
public:
    explicit ObjectManager(std::size_t max_object_count);

    ~ObjectManager();

private:
    std::size_t max_object_count;

    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> transforms;
};

#endif // EDITOR_OBJECT_MANAGER_H
