#ifndef NEW_SKYBOX_H
#define NEW_SKYBOX_H

#include "datetime.h"

#include <vsg/core/Array2D.h>
#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>

#include <chrono>
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
    /// hd_textures — пресеты High/Ultra (ТЗ "Графика"): предпочитать
    /// HD-варианты текстур "<имя>_hd.<ext>"; при отсутствии файла
    /// молча используется обычная текстура
    NewSkybox(const std::string& skybox_config_filepath,
              vsg::ref_ptr<vsg::Options> options = {},
              bool hd_textures = false);

    vsg::ref_ptr<vsg::Node> getNode() const;

    void set_date_time(const simulator_time_t& sim_time);
    void set_sun_direction(double azimuth_degrees, double altitude_degrees);

    /// Туман (ТЗ "Видимость и погода"): плотность, 1/м. Градиент
    /// неба затухает к цвету тумана (сильнее у горизонта)
    void set_fog(double fog_density);

private:
    void init_model(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    void init_textures(CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

private:
    bool is_sun_rise = false;

    /// Prefer HD texture variants "<name>_hd.<ext>" (High/Ultra presets)
    bool use_hd_textures = false;

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

    /// Туман: rgb - цвет, a - плотность, 1/м (uniform шейдера неба)
    vsg::ref_ptr<vsg::vec4Value> fog_value;

    /// Последняя установленная плотность тумана (пропуск лишних dirty)
    double last_fog_density = -1.0;

    /// Момент последнего обновления направления солнца.
    /// Член класса (не static): у каждого скайбокса свой таймер,
    /// иначе несколько экземпляров конфликтуют, а после паузы
    /// первое обновление пропускалось общим счётчиком
    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
};

#endif // NEW_SKYBOX_H
