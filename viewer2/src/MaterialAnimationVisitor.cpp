#include "MaterialAnimationVisitor.h"

#include "MaterialAnimation.h"
#include "ProcAnimation.h"

#include <vsg/core/Data.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/Node.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/material.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <mutex>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FindMaterialAnimationVisitor::FindMaterialAnimationVisitor(const FindMaterialAnimationVisitorCreateInfo& create_info)
    : pdo(create_info.pdo)
    , duplicate(create_info.duplicate)
    , cfg_reader(create_info.cfg_reader)
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
                    std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
                    pdo->tag(pbr_material_value);

                    auto new_pbr_material_value = vsg::PbrMaterialValue::create(*pbr_material_value);
                    new_pbr_material_value->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
                    duplicate->insert(pbr_material_value, new_pbr_material_value);

                    animation = ProcMaterialAnimation::create(new_pbr_material_value);
                    animation->load(cfg_reader);
                }
            }
        }
    }
}
