#ifndef EDITOR_STATIC_OBJECT_MANAGER_H
#define EDITOR_STATIC_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <cstddef>
#include <string>
#include <vector>

struct EditorContext;

namespace vsg
{

class MatrixTransform;
class PagedLOD;
class Switch;

}

class StaticObjectManager
{
public:
    StaticObjectManager(EditorContext& editor_ctx,
                        std::size_t max_object_count);

    bool add_object(const std::string& label, const std::string& rel_path,
                    const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
                    const vsg::dvec3& pos, const vsg::dvec3& rot_deg,
                    const vsg::dvec3& scale, bool is_selected = false,
                    bool is_hidden = false);

    bool remove_object(std::size_t id);

private:
    EditorContext& editor_ctx;
    std::vector<std::string> labels;
    std::vector<std::string> rel_paths;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> mat_transforms;
    std::vector<vsg::ref_ptr<vsg::Switch>> object_switches;
    std::vector<vsg::dmat4> init_mats;
    std::vector<bool> is_selected;
    std::vector<bool> is_hidden;
    std::size_t object_count;
    const std::size_t max_object_count;
};

#endif // EDITOR_STATIC_OBJECT_MANAGER_H
