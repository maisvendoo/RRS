#include "FindMaterialAnimationVisitor.h"

#include "ProcMaterialAnimation.h"

#include <vsg/core/Data.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/material.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <mutex>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FindMaterialAnimationVisitor::FindMaterialAnimationVisitor(vsg::ref_ptr<vsg::PropagateDynamicObjects> in_pdo,
                                                           vsg::ref_ptr<vsg::Duplicate> in_duplicate)
    : pdo(in_pdo)
    , duplicate(in_duplicate)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindMaterialAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindMaterialAnimationVisitor::apply(vsg::BindDescriptorSet& bindDescriptorSet)
{
    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto descriptor_buffer = descriptor.cast<vsg::DescriptorBuffer>())
        {
            for (auto& buffer_info : descriptor_buffer->bufferInfoList)
            {
                if (auto pbr_material_value = buffer_info->data.cast<vsg::PbrMaterialValue>())
                {
                    // Нашли материал, создаём новый
                    auto new_pbr_material_value = vsg::PbrMaterialValue::create(*pbr_material_value);
                    new_pbr_material_value->properties.dataVariance = vsg::DYNAMIC_DATA;

                    animation = ProcMaterialAnimation::create(new_pbr_material_value);
                    if (animation)
                    {
                        std::scoped_lock pdo_lock(pdo->mutex);
                        pdo->tag(pbr_material_value);
                        duplicate->insert(pbr_material_value, new_pbr_material_value);
                    }
                }
            }
        }
    }
}
