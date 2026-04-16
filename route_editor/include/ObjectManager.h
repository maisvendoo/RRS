#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "SingleSwitch.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <cstddef>
#include <string>
#include <vector>

struct EditorContext;

class ObjectManager
{
public:
    ObjectManager(EditorContext& context, std::size_t max_object_count);
    ~ObjectManager();

private:
    EditorContext& context_;

    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> transforms_;

    std::vector<std::string> labels_;

    std::vector<vsg::dvec3> translations_;
    std::vector<vsg::dvec3> rotation_degs_;
    std::vector<vsg::dvec3> scales_;

    std::vector<vsg::dmat4> initial_matrices_;
    std::vector<vsg::dbox> bounds_;

    std::vector<bool> is_selected_;
    std::vector<bool> is_hidden_;

    std::vector<vsg::ref_ptr<SingleSwitch>> paged_lod_switches_;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods_;
    std::vector<vsg::ref_ptr<SingleSwitch>> outline_switches_;
};

#endif // OBJECT_MANAGER_H
