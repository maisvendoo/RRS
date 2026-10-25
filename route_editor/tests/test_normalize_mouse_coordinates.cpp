#include "editor/editor_math.h"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdlib>

int main()
{
    VkExtent2D extent = {800, 600};
    double norm_x, norm_y;
    normalize_mouse_coordinates(400, 300, extent, norm_x, norm_y);
    assert(std::abs(norm_x) < 1.0e-6);
    assert(std::abs(norm_y) < 1.0e-6);
    return EXIT_SUCCESS;
}
