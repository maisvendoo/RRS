#ifndef STATIONS_HANDLER_H
#define STATIONS_HANDLER_H

#include <topology-types.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

class QByteArray;

namespace vsg
{
    class Options;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StationsHandler final
{
public:
    StationsHandler() = default;
    ~StationsHandler() = default;

    bool load(QByteArray& stations_data, vsg::ref_ptr<vsg::Options> options);

    /// Корневой узел графа сцены станций
    vsg::ref_ptr<vsg::Switch> getRootNode() const
    {
        return root;
    }

private:
    void deserialize(QByteArray& data);

    void createSceneGraph(vsg::ref_ptr<vsg::Options> options);

    /// Список станций
    topology_stations_list_t stations;

    /// Корневой узел станций (для включения/отключения отображения)
    vsg::ref_ptr<vsg::Switch> root;
};

#endif // STATIONS_HANDLER_H
