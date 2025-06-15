#include "FindDisplayAnimationVisitor.h"

#include "ProcAnimation.h"
#include "ProcDisplayAnimation.h"

#include "Logger.h"

#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/material.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/Image.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <mutex>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FindDisplayAnimationVisitor::FindDisplayAnimationVisitor(vsg::ref_ptr<vsg::PropagateDynamicObjects> in_pdo,
                                                         vsg::ref_ptr<vsg::Duplicate> in_duplicate)
    : pdo(in_pdo)
    , duplicate(in_duplicate)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindDisplayAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindDisplayAnimationVisitor::apply(vsg::BindDescriptorSet &bindDescriptorSet)
{
    LOG_INFO("Found bindDescriptorSet for display");

    // Текстура
    vsg::ref_ptr<vsg::Image> image_data = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> pixels = nullptr;
    // Материал
    vsg::ref_ptr<vsg::PbrMaterialValue> material_data = nullptr;

    bool image_not_found = true; // ищем первую из текстурных карт
    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto descriptor_image = descriptor.cast<vsg::DescriptorImage>())
        {
            if (image_not_found && (descriptor_image->imageInfoList.size() > 0))
            {
                if (auto image_info = descriptor_image->imageInfoList[0])
                {
                    if (auto image_view = image_info->imageView)
                    {
                        if (auto image = image_view->image)
                        {
                            image_data = image;
                            image_not_found = false;
                        }
                    }
                }
            }
        }
        if (auto descriptor_buffer = descriptor.cast<vsg::DescriptorBuffer>())
        {
            for (auto& buffer_info : descriptor_buffer->bufferInfoList)
            {
                if (auto pbr_material_value = buffer_info->data.cast<vsg::PbrMaterialValue>())
                {
                    material_data = pbr_material_value;
                }
            }
        }
    }

    pixels = image_data->data.cast<vsg::ubvec4Array2D>();

    if (!image_data || !pixels || !material_data)
    {
        if (!image_data)
            LOG_INFO("Not found image for display");
        if (!pixels)
            LOG_INFO("Not found pixels for display");
        if (!material_data)
            LOG_INFO("Not found material for display");
        return;
    }

    // Новая текстура
    vsg::ref_ptr<vsg::Image> new_image_data = vsg::Image::create(*image_data);
    new_image_data->data = vsg::ubvec4Array2D::create(*pixels);
    new_image_data->data->properties.dataVariance = vsg::DYNAMIC_DATA;
    // Новый материал
    vsg::ref_ptr<vsg::PbrMaterialValue> new_material_data = vsg::PbrMaterialValue::create(*material_data);
    new_material_data->properties.dataVariance = vsg::DYNAMIC_DATA;

    animation = ProcDisplayAnimation::create(new_image_data, new_material_data);
    if (animation)
    {
        std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
        pdo->tag(image_data);
        pdo->tag(material_data);
        duplicate->insert(image_data, new_image_data);
        duplicate->insert(material_data, new_material_data);
    }
}
