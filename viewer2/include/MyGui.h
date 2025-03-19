#ifndef MY_GUI_H
#define MY_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Group.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

struct Params : public vsg::Inherit<vsg::Object, Params>
{
    bool showGui = true;
    bool showDemoWindow = false;
    vsg::Group::Children route_models;
};

class MyGui : public vsg::Inherit<vsg::Command, MyGui>
{
public:
    vsg::ref_ptr<Params> params;

    MyGui(vsg::ref_ptr<Params> in_params, vsg::ref_ptr<vsg::Options> options = {});

    void compile(vsg::Context& context) override;

    void record(vsg::CommandBuffer& cb) const override;
};

#endif // MY_GUI_H
