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
    Skybox(std::string& skybox_model_filepath,
           std::vector<std::string> skybox_texture_filepath = {},
           vsg::ref_ptr<vsg::Options> options = {});

    /// Get scene node
    vsg::ref_ptr<vsg::Node> getNode() const noexcept;

    /// Get default texture
    vsg::ref_ptr<vsg::ubvec4Array2D> getDefaultTexture() const noexcept;

    /// Get textures array
    std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> getTextures() const noexcept;

private:
    vsg::ref_ptr<vsg::Node> node = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> texture = {};
    std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> textures = {};

    void init_model(std::string& skybox_model_filepath,
                    vsg::ref_ptr<vsg::Options> options);
    void init_textures(std::vector<std::string> skybox_texture_filepath,
                       vsg::ref_ptr<vsg::Options> options);
};

#endif // SKYBOX_H
