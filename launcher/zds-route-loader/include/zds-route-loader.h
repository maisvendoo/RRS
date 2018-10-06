#ifndef     ZDS_ROUTE_LOADER_H
#define     ZDS_ROUTE_LOADER_H

#include    "route-loader.h"
#include    "filesystem.h"
#include    "track.h"
#include    "object.h"

#include    <QMap>

class ZdsRouteLoader : public RouteLoader
{
    Q_OBJECT

public:

    ZdsRouteLoader(QObject *parent = Q_NULLPTR);

    ~ZdsRouteLoader();

    osg::Group   *load(QString route_path);

    osg::Vec3f getPosition(float rail_coord);

private:

    /// Route root directory
    QString     routeRootPath;

    /// Tracks list
    QMap<int, tracks_data_t>    tracks;
    /// Objects refs list
    QMap<QString, object_ref_t>     objects_refs;
    /// Object from objects.data list
    QList<objects_dat_t>            objects_dat;

    QList<object_map_t>             objects_map;

    QList<object_map_t>             sort_objects_map;

    QMap<QString, node_t>           nodes;

    /// Load 3D-model geometry
    osg::Node *loadModel(QString model_path, QString texture_path);

    /// Load 3D-model texture
    void loadTexture(QString texture_path,
                     osg::StateSet *stateset,
                     const unsigned int texture_unit);

    /// Find tracks description files (*.trk)
    QStringList findTrackFiles(QString route_dir_path);

    /// Load route tracks
    void loadTracks(QString route_dir_path);

    /// Load track data from *.trk file
    tracks_data_t loadTrackFile(QString path);

    /// Search track by railway coord
    track_t trackSearch(int track_idx, float rail_coord);

    /// Load objects referecies
    void loadObjectRefs(QString path);

    /// Load objects from objects.dat
    void loadObjectsDat(QString path);

    /// Load objects from route1.map
    void loadRoute1Map(QString path);

    void loadObjectsDatNodes(osg::Group *root_node);

    void loadRoute1MapNodes(osg::Group *group);

    void createSortedObjectsList();

    osg::Vec3f getPosition(float rail_coord, track_t &track);
};

#endif // ZDS_ROUTE_LOADER_H
