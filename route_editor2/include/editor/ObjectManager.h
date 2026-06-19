#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <vector>

namespace vsg
{

class MatrixTransform;
class PagedLOD;

}

class ObjectManager
{
public:
    explicit ObjectManager(int max_object_count);

    ~ObjectManager();

private:
    int max_object_count;

    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> transforms;
};

#endif // EDITOR_OBJECT_MANAGER_H
