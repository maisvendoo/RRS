#ifndef STATIONS_HANDLER_H
#define STATIONS_HANDLER_H

#include <topology-types.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

class QByteArray;

struct settings_t;

namespace vsg
{
    class Options;
    class Switch;
    class Node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StationsHandler final
{
public:
    explicit StationsHandler(const settings_t& settings);
    ~StationsHandler() = default;

    bool load(QByteArray& stations_data, vsg::ref_ptr<vsg::Options> options);

    /// Корневой узел графа сцены станций
    vsg::ref_ptr<vsg::Node> getRootNode() const;

    /// Включение/выключение отображения подписей станций
    void setVisible(bool visible);

private:

    void deserialize(QByteArray& data);

    bool createSceneGraph(vsg::ref_ptr<vsg::Options> options);

    /// Размер шрифта подписей станций, м
    double stations_text_font_size = 10.0;

    /// Смещение подписей станций относительно позиции станции, м
    vsg::dvec3 stations_text_shift = {0.0, 0.0, 15.0};

    /// Дистанция, после которой подписи станций уменьшаются с расстоянием, м
    double stations_text_scale_distance = 500.0;

    /// Список станций
    topology_stations_list_t stations;

    /// Корневой узел станций (для включения/отключения отображения)
    vsg::ref_ptr<vsg::Switch> root;
};

#endif // STATIONS_HANDLER_H
