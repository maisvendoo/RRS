#ifndef     ZDS_ROUTE_LOADER_H
#define     ZDS_ROUTE_LOADER_H

#include    "route-loader.h"
#include    "filesystem.h"

class ZdsRouteLoader : public RouteLoader
{
    Q_OBJECT

public:

    ZdsRouteLoader(QObject *parent = Q_NULLPTR);

    ~ZdsRouteLoader();

    osg::Node   *load(QString route_path);

private:

    QString     routeRootPath;

    osg::Node *loadModel(QString model_path, QString texture_path);

    void loadTexture(QString texture_path,
                     osg::StateSet *stateset,
                     const unsigned int texture_unit);

};

#endif // ZDS_ROUTE_LOADER_H
