#ifndef NEW_SKYBOX_H
#define NEW_SKYBOX_H

#include "datetime.h"

#include <vsg/core/Array2D.h>
#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

class CfgReader;

namespace vsg
{

class Data;
class Options;
class Node;
class StateGroup;
class MatrixTransform;

}

class NewSkybox
{
public:
    NewSkybox(const std::string& skybox_config_filepath, vsg::ref_ptr<vsg::Options> options = {});

    vsg::ref_ptr<vsg::Node> getNode() const;

    void set_date_time(const simulator_time_t& sim_time);
    void set_sun_direction(double azimuth_degrees, double altitude_degrees);

private:
    void init_model(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    void init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

private:
    bool is_sun_rise = false;

    struct texture_t
    {
        double angle_appear_begin = 0.0f;    ///< Возвышение солнца в начале плавного появления данной текстуры
        double angle_appear_end = 0.0f;      ///< Возвышение солнца в конце плавного появления данной текстуры
        double angle_disappear_begin = 0.0f; ///< Возвышение солнца в начале плавного исчезновения данной текстуры
        double angle_disappear_end = 0.0f;   ///< Возвышение солнца в конце плавного исчезновения данной текстуры
        float mix_value = 0.0f;
        int use_id = 0;

        vsg::ref_ptr<vsg::ubvec4Array2D> texture;       ///< Указатель на загруженную текстуру
        std::string filename;                           ///< Имя файла текстуры
    };

    std::vector<texture_t> textures;
    vsg::ref_ptr<vsg::StateGroup> state_group;
    vsg::ref_ptr<vsg::MatrixTransform> transform;
    vsg::ref_ptr<vsg::Data> texture1_data;
    vsg::ref_ptr<vsg::Data> texture2_data;
    vsg::ref_ptr<vsg::floatValue> mix_value;
};

#endif // NEW_SKYBOX_H
