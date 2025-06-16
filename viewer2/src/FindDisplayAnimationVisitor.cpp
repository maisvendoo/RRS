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

    // Контейнеры с текстурами
    vsg::ref_ptr<vsg::DescriptorImage> color_data_descriptor = nullptr;
    vsg::ref_ptr<vsg::DescriptorImage> emissive_data_descriptor = nullptr;
    // Материал
    vsg::ref_ptr<vsg::PbrMaterialValue> material_data = nullptr;

    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto descriptor_image = descriptor.cast<vsg::DescriptorImage>())
        {
            if (descriptor_image->dstBinding == 0)
            {
                // Нашли контейнер с текстурой базвого цвета
                color_data_descriptor = descriptor_image;
            }
            if (descriptor_image->dstBinding == 4)
            {
                // Нашли контейнер с текстурой цвета эмиссии
                emissive_data_descriptor = descriptor_image;
            }
        }
        if (auto descriptor_buffer = descriptor.cast<vsg::DescriptorBuffer>())
        {
            for (auto& buffer_info : descriptor_buffer->bufferInfoList)
            {
                if (auto pbr_material_value = buffer_info->data.cast<vsg::PbrMaterialValue>())
                {
                    // Нашли материал
                    material_data = pbr_material_value;
                }
            }
        }
    }

    if (!(color_data_descriptor || emissive_data_descriptor) || !material_data)
    {
        if (!color_data_descriptor)
            LOG_WARN("Not found base color descriptor for display");
        if (!emissive_data_descriptor)
            LOG_WARN("Not found emissive color descriptor for display");
        if (!material_data)
            LOG_WARN("Not found material for display");
        return;
    }

    // Текстура
    vsg::ref_ptr<vsg::Image> image_data = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> pixels = nullptr;

    bool is_color_image = false;
    if (color_data_descriptor && (color_data_descriptor->imageInfoList.size() > 0))
    {
        if (auto image_info = color_data_descriptor->imageInfoList[0])
        {
            if (auto image_view = image_info->imageView)
            {
                if (auto image = image_view->image)
                {
                    is_color_image = true;
                    image_data = image;
                }
            }
        }
    }

    bool is_emissive_image = false;
    if (emissive_data_descriptor && (emissive_data_descriptor->imageInfoList.size() > 0))
    {
        if (auto image_info = emissive_data_descriptor->imageInfoList[0])
        {
            if (auto image_view = image_info->imageView)
            {
                if (auto image = image_view->image)
                {
                    is_emissive_image = true;
                    if (image_data)
                    {
                        if (image_data != image)
                        {
                            LOG_WARN("Emissive texture is different from base color texture");
                        }
                    }
                    else
                    {
                        image_data = image;
                        LOG_WARN("Base color texture is not found. Using emissive texture");
                    }
                }
            }
        }
    }

    pixels = image_data->data.cast<vsg::ubvec4Array2D>();

    if (!image_data || !pixels)
    {
        if (!image_data)
            LOG_WARN("Not found image for display");
        if (!pixels)
            LOG_WARN("Not found RGBA pixels for display");
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

        if (is_color_image)
        {
            vsg::ref_ptr<vsg::ImageView> new_color_data_image_view = vsg::ImageView::create(*(color_data_descriptor->imageInfoList[0]->imageView));
            new_color_data_image_view->image = new_image_data;

            // ImageInfo has deleted copy constructor
            vsg::ref_ptr<vsg::ImageInfo> new_color_data_image_info = vsg::ImageInfo::create(color_data_descriptor->imageInfoList[0]->sampler,
                                                                                            new_color_data_image_view,
                                                                                            color_data_descriptor->imageInfoList[0]->imageLayout);

            vsg::ref_ptr<vsg::DescriptorImage> new_color_data_descriptor = vsg::DescriptorImage::create(*color_data_descriptor);
            new_color_data_descriptor->imageInfoList[0] = new_color_data_image_info;

            pdo->tag(color_data_descriptor);
            duplicate->insert(color_data_descriptor, new_color_data_descriptor);
        }
        if (is_emissive_image)
        {
            vsg::ref_ptr<vsg::ImageView> new_emissive_data_image_view = vsg::ImageView::create(*(emissive_data_descriptor->imageInfoList[0]->imageView));
            new_emissive_data_image_view->image = new_image_data;

            // ImageInfo has deleted copy constructor
            vsg::ref_ptr<vsg::ImageInfo> new_emissive_data_image_info = vsg::ImageInfo::create(emissive_data_descriptor->imageInfoList[0]->sampler,
                                                                                               new_emissive_data_image_view,
                                                                                               emissive_data_descriptor->imageInfoList[0]->imageLayout);

            vsg::ref_ptr<vsg::DescriptorImage> new_emissive_data_descriptor = vsg::DescriptorImage::create(*emissive_data_descriptor);
            new_emissive_data_descriptor->imageInfoList[0] = new_emissive_data_image_info;

            pdo->tag(emissive_data_descriptor);
            duplicate->insert(emissive_data_descriptor, new_emissive_data_descriptor);
        }

        pdo->tag(material_data);
        duplicate->insert(material_data, new_material_data);
    }
}
