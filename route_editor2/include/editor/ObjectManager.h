#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <string>
#include <vector>

class Camera;

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

    void push_label(const std::string& label);

    void push_path(const std::string& path);

    void push_paged_lod(const vsg::ref_ptr<vsg::PagedLOD>& paged_lod);

    void push_matrix_transform(const vsg::ref_ptr<vsg::MatrixTransform>& matrix_transform);

    const std::vector<vsg::ref_ptr<vsg::MatrixTransform>>& get_transforms() const;

    void check_intersections_and_select_objects(const vsg::ref_ptr<Camera>& camera,
        int x1, int y1, int x2, int y2);

private:
    std::size_t max_object_count;

    std::vector<std::string> labels;
    std::vector<std::string> paths;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> transforms;
};

#endif // EDITOR_OBJECT_MANAGER_H
