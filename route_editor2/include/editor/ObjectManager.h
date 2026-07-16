#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <string>
#include <vector>

namespace vsg
{

class Commands;
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

    void push_draw_commands_for_selection(const vsg::ref_ptr<vsg::Commands>& commands);

    const std::vector<vsg::ref_ptr<vsg::MatrixTransform>>& get_transforms() const;

// private:
    std::size_t max_object_count;

    std::vector<std::string> labels;
    std::vector<std::string> paths;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> transforms;
    std::vector<vsg::ref_ptr<vsg::Commands>> draw_commands_for_selection;
    std::vector<bool> is_selected;
};

#endif // EDITOR_OBJECT_MANAGER_H
