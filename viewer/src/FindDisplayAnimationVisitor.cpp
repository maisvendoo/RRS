#include "FindDisplayAnimationVisitor.h"

#include "Logger.h"
#include "ProcDisplayAnimation.h"

#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/material.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
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
    vsg::ref_ptr<vsg::DescriptorImage> color_descriptor = nullptr;
    vsg::ref_ptr<vsg::DescriptorImage> emissive_descriptor = nullptr;
    // Материал
    vsg::ref_ptr<vsg::PbrMaterialValue> material_data = nullptr;

    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto descriptor_image = descriptor.cast<vsg::DescriptorImage>())
        {
            if (descriptor_image->dstBinding == 0)
            {
                // Нашли контейнер с текстурой базвого цвета
                color_descriptor = descriptor_image;
            }
            if (descriptor_image->dstBinding == 4)
            {
                // Нашли контейнер с текстурой цвета эмиссии
                emissive_descriptor = descriptor_image;
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

    if (!(color_descriptor || emissive_descriptor) || !material_data)
    {
        if (!color_descriptor)
            LOG_WARN("Not found base color descriptor for display");
        if (!emissive_descriptor)
            LOG_WARN("Not found emissive color descriptor for display");
        if (!material_data)
            LOG_WARN("Not found material for display");
        return;
    }

    // Текстура цвета
    vsg::ref_ptr<vsg::Image> color_image = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> color_pixels = nullptr;
    if (color_descriptor && (color_descriptor->imageInfoList.size() > 0))
    {
        if (auto image_info = color_descriptor->imageInfoList[0])
        {
            if (auto image_view = image_info->imageView)
            {
                if (auto image = image_view->image)
                {
                    color_image = image;
                    color_pixels = image->data.cast<vsg::ubvec4Array2D>();
                }
            }
        }
    }

    // Текстура эмиссии
    vsg::ref_ptr<vsg::Image> emissive_image = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> emissive_pixels = nullptr;
    if (emissive_descriptor && (emissive_descriptor->imageInfoList.size() > 0))
    {
        if (auto image_info = emissive_descriptor->imageInfoList[0])
        {
            if (auto image_view = image_info->imageView)
            {
                if (auto image = image_view->image)
                {
                    emissive_image = image;
                    emissive_pixels = image->data.cast<vsg::ubvec4Array2D>();
                }
            }
        }
    }

    if (!(color_image && color_pixels))
    {
        LOG_WARN("Not found color image for display");
        color_image = nullptr;
    }
    if (!(emissive_image && emissive_pixels))
    {
        LOG_WARN("Not found emissive image for display");
        emissive_image = nullptr;
    }
    if (!(color_image || emissive_image))
    {
        return;
    }

    // Создаём дубликаты для анимации
    // Новая текстура цвета
    vsg::ref_ptr<vsg::Image> new_color_image = nullptr;
    if (color_image)
    {
        new_color_image = vsg::Image::create(*color_image);
    }
    // Новая текстура эмиссии
    vsg::ref_ptr<vsg::Image> new_emissive_image = nullptr;
    if (emissive_image)
    {
        new_emissive_image = vsg::Image::create(*emissive_image);
    }
    // Новый материал
    vsg::ref_ptr<vsg::PbrMaterialValue> new_material_data = vsg::PbrMaterialValue::create(*material_data);

    // Создаём анимацию
    animation = ProcDisplayAnimation::create(new_color_image, new_emissive_image, new_material_data);
    if (animation)
    {
        std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);

        if (new_color_image)
        {
            vsg::ref_ptr<vsg::ImageView> new_color_image_view = vsg::ImageView::create(*(color_descriptor->imageInfoList[0]->imageView));
            new_color_image_view->image = new_color_image;

            // ImageInfo has deleted copy constructor
            vsg::ref_ptr<vsg::ImageInfo> new_color_image_info = vsg::ImageInfo::create(color_descriptor->imageInfoList[0]->sampler,
                                                                                       new_color_image_view,
                                                                                       color_descriptor->imageInfoList[0]->imageLayout);

            vsg::ref_ptr<vsg::DescriptorImage> new_color_descriptor = vsg::DescriptorImage::create(*color_descriptor);
            new_color_descriptor->imageInfoList[0] = new_color_image_info;

            pdo->tag(color_descriptor);
            duplicate->insert(color_descriptor, new_color_descriptor);
        }
        if (new_emissive_image)
        {
            vsg::ref_ptr<vsg::ImageView> new_emissive_image_view = vsg::ImageView::create(*(emissive_descriptor->imageInfoList[0]->imageView));
            new_emissive_image_view->image = new_emissive_image;

            // ImageInfo has deleted copy constructor
            vsg::ref_ptr<vsg::ImageInfo> new_emissive_image_info = vsg::ImageInfo::create(emissive_descriptor->imageInfoList[0]->sampler,
                                                                                          new_emissive_image_view,
                                                                                          emissive_descriptor->imageInfoList[0]->imageLayout);

            vsg::ref_ptr<vsg::DescriptorImage> new_emissive_descriptor = vsg::DescriptorImage::create(*emissive_descriptor);
            new_emissive_descriptor->imageInfoList[0] = new_emissive_image_info;

            pdo->tag(emissive_descriptor);
            duplicate->insert(emissive_descriptor, new_emissive_descriptor);
        }

        pdo->tag(material_data);
        duplicate->insert(material_data, new_material_data);
    }
}
