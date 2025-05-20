#include "MaterialAnimationVisitor.h"

#include "animations-list.h"
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

MaterialAnimationVisitor::MaterialAnimationVisitor(const MaterialAnimationVisitorCreateInfo& create_info)
    : pdo(create_info.pdo)
    , duplicate(create_info.duplicate)
    , animations(create_info.animations)
    , cfg_reader(create_info.cfg_reader)
{
}

void MaterialAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void MaterialAnimationVisitor::apply(vsg::BindDescriptorSet& bindDescriptorSet)
{
    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto* descriptor_buffer = descriptor->cast<vsg::DescriptorBuffer>())
        {
            for (auto& buffer_info : descriptor_buffer->bufferInfoList)
            {
                if (auto* pbr_material_value = buffer_info->data->cast<vsg::PbrMaterialValue>())
                {
                    std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
                    pdo->tag(pbr_material_value);

                    auto new_pbr_material_value = vsg::PbrMaterialValue::create(*pbr_material_value);
                    new_pbr_material_value->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
                    duplicate->insert(pbr_material_value, new_pbr_material_value);

                    animation = new MaterialAnimation(new_pbr_material_value);
                    animation->load(cfg_reader);
                    // animations->thread_safe_insert({animation->getSignalID(), animation});
                }
            }
        }
    }
}
