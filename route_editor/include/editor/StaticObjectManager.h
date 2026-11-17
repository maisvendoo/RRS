#ifndef EDITOR_STATIC_OBJECT_MANAGER_H
#define EDITOR_STATIC_OBJECT_MANAGER_H

#include "editor/SparseSet.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Switch.h>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

struct EditorContext;

namespace vsg
{

class MatrixTransform;
class PagedLOD;

}

class StaticObjectManager
{
public:
    StaticObjectManager(EditorContext& editor_ctx,
                        std::size_t max_object_count);

    bool add_object(const std::string& label, const std::string& rel_path,
                    const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
                    const vsg::dvec3& pos, const vsg::dvec3& rot_deg,
                    const vsg::dvec3& scale);

    void remove_object(std::size_t id);

    bool select_object(std::size_t id);
    bool select_object(const vsg::MatrixTransform* mat_transform);

    void deselect_object(std::size_t id);

    void set_position(std::size_t id, const vsg::dvec3& pos);
    void move_object(std::size_t id, const vsg::dvec3& delta_pos);

    const SparseSet& get_selected_objects() const;

    std::size_t get_object_count() const;

private:
    EditorContext& editor_ctx;

    std::vector<std::string> labels;
    std::vector<std::string> rel_paths; // Replace on absolute?

    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> mat_transforms;
    std::vector<vsg::ref_ptr<vsg::Switch>> vsg_switches;
    std::vector<vsg::Switch::Child*> paged_lod_switches;
    std::vector<vsg::Switch::Child*> outline_switches;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;

    // Vector init_mats is used to store the initial matrices
    // when the command is canceled
    std::vector<vsg::dmat4> init_mats;

    std::map<const vsg::MatrixTransform*, std::size_t/* id*/>
    mat_transform_to_id;

    SparseSet selected_objects;
    SparseSet hidden_objects;

    std::size_t object_count;
    const std::size_t max_object_count;
};

#endif // EDITOR_STATIC_OBJECT_MANAGER_H
