#ifndef NEW_SKYBOX_H
#define NEW_SKYBOX_H

#include "datetime.h"

#include <vsg/core/Array2D.h>
#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>

#include <cstdint>
#include <string>
#include <vector>

class CfgReader;

namespace vsg
{

class Data;
class Options;
class StateGroup;

}

class NewSkybox
{
public:
    NewSkybox(const std::string& skybox_config_filepath, vsg::ref_ptr<vsg::Options> options = {});

    vsg::ref_ptr<vsg::StateGroup> get_state_group() const;

    void set_date_time(const simulator_time_t& sim_time);

private:
    void init_model(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    void init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

private:
    struct season_time_texture_t
    {
        enum class State
        {
            ACTIVE,
            INACTIVE,
            APPEARING,
            DISAPPEARING
        } state;

        struct season_date_t {std::uint8_t month; std::uint8_t day;};
        season_date_t date_season_begin = {1, 1};       ///< День начала сезона применения данной текстуры (включительно)
        season_date_t date_season_end = {12, 31};       ///< День окончания сезона применения данной текстуры (включительно)

        server_time_t time_appear_begin = {0, 0, 0};    ///< Время начала плавного появления данной текстуры
        server_time_t time_appear_end = {0, 0, 0};      ///< Время окончания плавного появления данной текстуры
        server_time_t time_disappear_begin = {0, 0, 0}; ///< Время начала плавного исчезновения данной текстуры
        server_time_t time_disappear_end = {0, 0, 0};   ///< Время окончания плавного исчезновения данной текстуры

        vsg::ref_ptr<vsg::ubvec4Array2D> texture;       ///< Указатель на загруженную текстуру
        std::string filename;                           ///< Имя файла текстуры
    };

    std::vector<season_time_texture_t> textures;
    vsg::ref_ptr<vsg::StateGroup> state_group;
    vsg::ref_ptr<vsg::Data> texture1_data;
    vsg::ref_ptr<vsg::Data> texture2_data;
    vsg::ref_ptr<vsg::floatValue> mix_value;
};

#endif // NEW_SKYBOX_H
