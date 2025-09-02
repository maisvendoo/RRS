#ifndef SKYBOX_H
#define SKYBOX_H

#include "datetime.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

#include <cstdint>
#include <string>

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

    /// Set date and time
    void setDateTime(simulator_time_t sim_time);

    /// Set active textures and their weights
    void setActiveTextures(std::map<vsg::ref_ptr<vsg::ubvec4Array2D>, float> textures_and_weights);

private:
    vsg::ref_ptr<vsg::Node> node;
    vsg::ref_ptr<vsg::ubvec4Array2D> texture;
    std::map<vsg::ref_ptr<vsg::ubvec4Array2D>, float> active_textures_and_weights;

    struct season_time_texture_t
    {
        struct season_date_t {std::uint8_t month; std::uint8_t day;};
        season_date_t date_season_begin = {1, 1};       ///< День начала сезона применения данной текстуры (включительно)
        season_date_t date_season_end = {12, 31};       ///< День окончания сезона применения данной текстуры (включительно)

        server_time_t time_appear_begin = {0, 0, 0};    ///< Время начала плавного появления данной текстуры
        server_time_t time_appear_end = {0, 0, 0};      ///< Время окончания плавного появления данной текстуры
        server_time_t time_disappear_begin = {0, 0, 0}; ///< Время начала плавного исчезновения данной текстуры
        server_time_t time_disappear_end = {0, 0, 0};   ///< Время окончания плавного исчезновения данной текстуры

        vsg::ref_ptr<vsg::ubvec4Array2D> texture;       ///< Указатель на загруженную текстуру
        std::string filename;   ///< Имя файла текстуры
    };

    std::vector<season_time_texture_t> textures;

    void update_skybox();
    void init_model(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    void init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
};

#endif // SKYBOX_H
