// #include    "display-surface-visitor.h"

// #include    "QWidgetImage.h"
// #include    <osgViewer/ViewerEventHandlers>
// #include    <osg/Material>
// #include    <osg/Texture2D>

// DisplaySurfaceVisitor::DisplaySurfaceVisitor(display_container_t *dc, display_config_t display_config)
//     : osg::NodeVisitor()
//     , dc(dc)
//     , display_config(display_config)

// {

// }

// void DisplaySurfaceVisitor::apply(osg::Geode &geode)
// {
//     dc->widgetImage = new osgQt::QWidgetImage(dc->display);
//     dc->texture = new osg::Texture2D(dc->widgetImage.get());
//     dc->texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
//     dc->texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

//     osg::StateSet *stateset = geode.getOrCreateStateSet();
//     stateset->setTextureAttributeAndModes(0, dc->texture.get());

//     dc->handler = new osgViewer::InteractiveImageHandler(dc->widgetImage.get());

//     geode.getDrawable(0)->setEventCallback(dc->handler.get());
//     geode.getDrawable(0)->setCullCallback(dc->handler.get());

//     osg::Geometry *geom = static_cast<osg::Geometry *>(geode.getDrawable(0));

//     osg::Vec3Array *verts = (osg::Vec3Array *) geom->getVertexArray();

//     geom->setTexCoordArray(0, display_config.texcoord.get());

//     traverse(geode);
// }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "display-surface-visitor.h"
#include "display-config.h"

#include <iostream>
#include <qobjectdefs.h>
#include <qthread.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Data.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec4.h>

#include <QImage>
#include <QPainter>
#include <QApplication>

#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/Sampler.h>
#include <vulkan/vulkan_core.h>

DisplaySurfaceVisitor::DisplaySurfaceVisitor(display_container_t* dc, const display_config_t& display_config)
    : dc(dc)
    , display_config(display_config)
{
}

void DisplaySurfaceVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void DisplaySurfaceVisitor::apply(vsg::StateGroup& stateGroup)
{
    QMetaObject::invokeMethod(qApp, [&]() {
        std::cout << vsg::ref_ptr(&stateGroup) << std::endl;

        QImage image(dc->display->size(), QImage::Format_RGBA8888_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        dc->display->moveToThread(QThread::currentThread());
        dc->display->render(&painter);
        painter.end();

        vsg::ref_ptr<vsg::Data> vsgData;
        if (image.format() == QImage::Format_RGBA8888_Premultiplied)
        {
            vsgData = vsg::ubvec4Array2D::create(
                image.width(),
                image.height(),
                reinterpret_cast<vsg::ubvec4*>(image.bits()),
                vsg::Data::Layout{VK_FORMAT_R8G8B8A8_UNORM}
            );
        }
        else
        {
            QImage converted = image.convertToFormat(QImage::Format_RGBA8888);
            vsgData = vsg::ubvec4Array2D::create(
                converted.width(),
                converted.height(),
                reinterpret_cast<vsg::ubvec4*>(converted.bits()),
                vsg::Data::Layout{VK_FORMAT_R8G8B8A8_UNORM}
            );
        }
    }, Qt::BlockingQueuedConnection);

    // auto texture = vsg::DescriptorImage::create(
    //     vsg::Sampler::create(),
    //     vsgData
    // );

    // vsg::DescriptorSetLayoutBindings descriptorBindings{
    //     {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    // };

    // auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    // vsg::PushConstantRanges pushConstantRanges{
    //     {VK_SHADER_STAGE_VERTEX_BIT, 0, 128}
    // };

    // auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    // auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);

    // auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, vsg::DescriptorSets{descriptorSet});

    // stateGroup.add(bindDescriptorSets);
}
