#include "MaterialRgbAnimationVisitor.h"
#include "MaterialRgbAnimation.h"
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/material.h>

MaterialRgbAnimationVisitor::MaterialRgbAnimationVisitor(animations_t* animations, CfgReader &cfg)
    : vsg::Visitor()
    , animations(animations)
    , cfg(&cfg)
{

}

void MaterialRgbAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void MaterialRgbAnimationVisitor::apply(vsg::StateGroup& stateGroup)
{
    for (auto& command : stateGroup.stateCommands)
    {
        if (auto* bindDescriptorSet = command->cast<vsg::BindDescriptorSet>())
        {
            command = bindDescriptorSet->clone()->cast<vsg::BindDescriptorSet>();
            bindDescriptorSet = command->cast<vsg::BindDescriptorSet>();

            vsg::ref_ptr<vsg::DescriptorSet> descriptorSet(bindDescriptorSet->descriptorSet->clone()->cast<vsg::DescriptorSet>());
            bindDescriptorSet->descriptorSet = descriptorSet;

            for (auto& descriptor : descriptorSet->descriptors)
            {
                if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                {
                    descriptor = descriptorBuffer->clone()->cast<vsg::DescriptorBuffer>();
                    descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>();

                    auto material_value = vsg::PbrMaterialValue::create();
                    auto bufferInfo = vsg::BufferInfo::create(material_value);
                    descriptorBuffer->bufferInfoList.clear();
                    descriptorBuffer->bufferInfoList.push_back(bufferInfo);

                    auto& material = material_value->value();

                    ProcAnimation *animation = new MaterialRgbAnimation(material);
                    animation->load(*cfg);
                    animations->thread_safe_insert({animation->getSignalID(), animation});
                }
            }
        }
    }

    stateGroup.traverse(*this);
}
