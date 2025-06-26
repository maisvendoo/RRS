#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

namespace vsg
{
    class Options;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Skybox final
{
public:
    Skybox(std::string& skybox_model_filepath, vsg::ref_ptr<vsg::Options> options = {});

    /// Get scene node
    vsg::ref_ptr<vsg::Node> getNode();

private:
    vsg::ref_ptr<vsg::Node> node = nullptr;

    void init(std::string& skybox_model_filepath, vsg::ref_ptr<vsg::Options> options);
};

#endif // SKYBOX_H
