#include    "zds-route-loader.h"

ZdsRouteLoader::ZdsRouteLoader(QObject *parent) : RouteLoader(parent)
{

}

ZdsRouteLoader::~ZdsRouteLoader()
{

}

osg::Node *ZdsRouteLoader::load(QString route_path)
{
    osg::Node *routeRoot = new osg::Group;

    return routeRoot;
}

GET_ROUTE_LOADER(ZdsRouteLoader)
