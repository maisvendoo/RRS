#ifndef     ZDS_ROUTE_LOADER_H
#define     ZDS_ROUTE_LOADER_H

#include    "route-loader.h"
#include    "filesystem.h"
#include    "track.h"

#include    <QMap>

class ZdsRouteLoader : public RouteLoader
{
    Q_OBJECT

public:

    ZdsRouteLoader(QObject *parent = Q_NULLPTR);

    ~ZdsRouteLoader();

    osg::Node   *load(QString route_path);

private:

    QString     routeRootPath;

    QMap<int, tracks_data_t> tracks;

    osg::Node *loadModel(QString model_path, QString texture_path);

    void loadTexture(QString texture_path,
                     osg::StateSet *stateset,
                     const unsigned int texture_unit);

    QStringList findTrackFiles(QString route_dir_path);

    void loadTracks(QString route_dir_path);

    tracks_data_t loadTrackFile(QString path);
};

#endif // ZDS_ROUTE_LOADER_H
