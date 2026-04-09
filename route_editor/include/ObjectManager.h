#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <cstddef>
#include <string>

struct EditorContext;
class SingleSwitch;

namespace vsg
{

class MatrixTransform;
class PagedLOD;

}

class ObjectManager
{
public:
    explicit ObjectManager(EditorContext& context, std::size_t max_object_count);
    ~ObjectManager();

private:
    EditorContext& context;

    vsg::ref_ptr<vsg::MatrixTransform>* transforms = nullptr;

    std::string* labels = nullptr;
    vsg::dvec3* translations = nullptr;
    vsg::dvec3* rotation_degs = nullptr;
    vsg::dvec3* scales = nullptr;

    vsg::dmat4* initial_matrices = nullptr;
    vsg::dbox* bounds = nullptr;

    bool* is_selected = nullptr;
    bool* is_hidden = nullptr;

    vsg::ref_ptr<SingleSwitch>* paged_lod_switches = nullptr;
    vsg::ref_ptr<vsg::PagedLOD>* paged_lods = nullptr;
    vsg::ref_ptr<SingleSwitch>* outline_switches = nullptr;
};

#endif // OBJECT_MANAGER_H
