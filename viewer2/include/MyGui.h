#ifndef MY_GUI_H
#define MY_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <vector>

namespace vsg
{
    class CommandBuffer;
    class Context;
    class Node;
    class Options;
};

struct Params : public vsg::Inherit<vsg::Object, Params>
{
    Params();

    bool showGui;
    bool showDemoWindow;
    static std::vector<vsg::ref_ptr<vsg::Node>> nodes;
};

class MyGui : public vsg::Inherit<vsg::Command, MyGui>
{
public:
    vsg::ref_ptr<Params> params;

    MyGui(vsg::ref_ptr<Params> in_params, vsg::ref_ptr<vsg::Options> options = {});

    void compile(vsg::Context& context) override;

    void record(vsg::CommandBuffer& cb) const override;

private:
    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;
};

#endif // MY_GUI_H
