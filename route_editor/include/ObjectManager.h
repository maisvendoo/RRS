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
    EditorContext& context_;

    vsg::ref_ptr<vsg::MatrixTransform>* transforms_ = nullptr;

    std::string* labels_ = nullptr;
    vsg::dvec3* translations_ = nullptr;
    vsg::dvec3* rotation_degs_ = nullptr;
    vsg::dvec3* scales_ = nullptr;

    vsg::dmat4* initial_matrices_ = nullptr;
    vsg::dbox* bounds_ = nullptr;

    bool* is_selected_ = nullptr;
    bool* is_hidden_ = nullptr;

    vsg::ref_ptr<SingleSwitch>* paged_lod_switches_ = nullptr;
    vsg::ref_ptr<vsg::PagedLOD>* paged_lods_ = nullptr;
    vsg::ref_ptr<SingleSwitch>* outline_switches_ = nullptr;
};

#endif // OBJECT_MANAGER_H
