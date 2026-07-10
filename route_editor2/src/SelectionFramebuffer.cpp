#include "editor/SelectionFramebuffer.h"

#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/Sampler.h>
#include <vsg/utils/CoordinateSpace.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/RenderPass.h>

#include <vulkan/vulkan_core.h>

static vsg::ref_ptr<vsg::ImageInfo> create_color_image_info(
    const vsg::ref_ptr<vsg::Context>& context,
    VkExtent3D attachment_extent
);

static vsg::ref_ptr<vsg::ImageInfo> create_depth_image_info(
    const vsg::ref_ptr<vsg::Context>& context,
    VkExtent3D attachment_extent,
    VkFormat depth_format
);

static vsg::RenderPass::Attachments create_render_pass_attachments(VkFormat depth_format);

static vsg::RenderPass::Dependencies create_render_pass_dependencies();

SelectionFramebuffer::SelectionFramebuffer(
    const vsg::ref_ptr<vsg::Context>& context,
    VkExtent2D window_extent
)
{
    const auto& device = context->device;
    const VkExtent3D attachment_extent = {window_extent.width, window_extent.height, 1};
    const VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

    const auto color_image_info = create_color_image_info(context, attachment_extent);
    const auto depth_image_info = create_depth_image_info(context, attachment_extent, depth_format);

    const vsg::RenderPass::Attachments attachments = create_render_pass_attachments(depth_format);

    const vsg::AttachmentReference color_reference = {
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const vsg::AttachmentReference depth_reference = {
        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    vsg::RenderPass::Subpasses subpass_description(1);
    subpass_description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description[0].colorAttachments.emplace_back(color_reference);
    subpass_description[0].depthStencilAttachments.emplace_back(depth_reference);

    vsg::RenderPass::Dependencies dependencies = create_render_pass_dependencies();

    const auto render_pass = vsg::RenderPass::create(device, attachments,
        subpass_description, dependencies);

    const auto framebuffer = vsg::Framebuffer::create(render_pass,
        vsg::ImageViews{color_image_info->imageView, depth_image_info->imageView},
        window_extent.width, window_extent.height, 1);

    const auto render_graph = vsg::RenderGraph::create();
    render_graph->renderArea.offset = VkOffset2D{0, 0};
    render_graph->renderArea.extent = window_extent;
    render_graph->framebuffer = framebuffer;

    render_graph->clearValues.resize(2);
    render_graph->clearValues[0].color = vsg::sRGB_to_linear(0.4f, 0.2f, 0.4f, 1.0f);
    render_graph->clearValues[1].depthStencil = VkClearDepthStencilValue{0.0f, 0};

    // const auto view = vsg::View::create(camera, scenegraph);
    // render_graph->addChild(view);
}

vsg::ref_ptr<vsg::ImageInfo> create_color_image_info(
    const vsg::ref_ptr<vsg::Context>& context,
    VkExtent3D attachment_extent
)
{
    const auto color_image = vsg::Image::create();
    color_image->imageType = VK_IMAGE_TYPE_2D;
    color_image->format = VK_FORMAT_R8G8B8A8_SRGB;
    color_image->extent = attachment_extent;
    color_image->mipLevels = 1;
    color_image->arrayLayers = 1;
    color_image->samples = VK_SAMPLE_COUNT_1_BIT;
    color_image->tiling = VK_IMAGE_TILING_OPTIMAL;
    color_image->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    color_image->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image->flags = 0;
    color_image->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const auto color_image_view = vsg::createImageView(*context, color_image,
        VK_IMAGE_ASPECT_COLOR_BIT);

    const auto color_sampler = vsg::Sampler::create();
    color_sampler->flags = 0;
    color_sampler->magFilter = VK_FILTER_LINEAR;
    color_sampler->minFilter = VK_FILTER_LINEAR;
    color_sampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    color_sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    color_sampler->addressModeV = color_sampler->addressModeU;
    color_sampler->addressModeW = color_sampler->addressModeU;
    color_sampler->mipLodBias = 0.0f;
    color_sampler->maxAnisotropy = 1.0f;
    color_sampler->minLod = 0.0f;
    color_sampler->maxLod = 1.0f;
    color_sampler->borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    const auto color_image_info = vsg::ImageInfo::create();
    color_image_info->imageView = color_image_view;
    color_image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    color_image_info->sampler = color_sampler;

    return color_image_info;
}

vsg::ref_ptr<vsg::ImageInfo> create_depth_image_info(
    const vsg::ref_ptr<vsg::Context>& context,
    VkExtent3D attachment_extent,
    VkFormat depth_format
)
{
    const auto depth_image = vsg::Image::create();
    depth_image->imageType = VK_IMAGE_TYPE_2D;
    depth_image->extent = attachment_extent;
    depth_image->mipLevels = 1;
    depth_image->arrayLayers = 1;
    depth_image->samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image->format = depth_format;
    depth_image->tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_image->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_image->flags = 0;
    depth_image->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const auto depth_image_info = vsg::ImageInfo::create();
    depth_image_info->sampler = nullptr;
    depth_image_info->imageView = vsg::createImageView(*context, depth_image,
        VK_IMAGE_ASPECT_DEPTH_BIT);
    depth_image_info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    return depth_image_info;
}

vsg::RenderPass::Attachments create_render_pass_attachments(VkFormat depth_format)
{
    vsg::RenderPass::Attachments attachments(2);

    attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format = depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return attachments;
}

vsg::RenderPass::Dependencies create_render_pass_dependencies()
{
    vsg::RenderPass::Dependencies dependencies(2);

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return dependencies;
}
