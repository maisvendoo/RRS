#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "datetime.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

class CfgReader;
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
    Skybox(const std::string& skybox_config_filepath,
           vsg::ref_ptr<vsg::Options> options = {});

    /// Get scene node
    vsg::ref_ptr<vsg::Node> getNode() const noexcept;

    /// Get default texture
    vsg::ref_ptr<vsg::ubvec4Array2D> getDefaultTexture() const noexcept;

    /// Get textures array
    std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> getTextures() const noexcept;

private:
    vsg::ref_ptr<vsg::Node> node = nullptr;
    vsg::ref_ptr<vsg::ubvec4Array2D> texture = nullptr;

    struct season_time_texture_t
    {
        struct season_date_t {uint8_t month; uint8_t day;};
        season_date_t date_season_begin = {1, 1};
        season_date_t date_season_end = {12, 31};
        bool is_season_trough_new_year = false;

        server_time_t time_appear_begin = {0, 0, 0};
        server_time_t time_appear_end = {0, 0, 0};
        server_time_t time_disappear_begin = {0, 0, 0};
        server_time_t time_disappear_end = {0, 0, 0};
        bool is_time_trough_midhight = false;

        vsg::ref_ptr<vsg::ubvec4Array2D> texture = nullptr;
        std::string filename = "";
    };

    std::vector<season_time_texture_t> textures = {};

    void init_model(CfgReader &cfg,
                    vsg::ref_ptr<vsg::Options> options);
    void init_textures(CfgReader &cfg,
                       vsg::ref_ptr<vsg::Options> options);
};

#endif // SKYBOX_H
