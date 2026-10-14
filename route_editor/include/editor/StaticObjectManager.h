#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>

#include <cstddef>
#include <string>
#include <vector>

namespace vsg
{

class MatrixTransform;

}

class StaticObjectManager
{
public:
    explicit StaticObjectManager(std::size_t max_object_count);

    bool add_object(const std::string& label, const std::string& rel_path,
                    const vsg::ref_ptr<vsg::MatrixTransform>& mat_transform,
                    bool is_selected = false, bool is_hidden = false);

    bool remove_object(std::size_t idx);

private:
    std::vector<std::string> labels;
    std::vector<std::string> rel_paths;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> mat_transforms;
    std::vector<vsg::dmat4> init_mats;
    std::vector<bool> is_selected;
    std::vector<bool> is_hidden;
    std::size_t object_count;
    const std::size_t max_object_count;
};

#endif // EDITOR_OBJECT_MANAGER_H
