#include    "route-loader.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader::RouteLoader(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader::~RouteLoader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteLoader::setFileSystem(FileSystem *fs)
{
    this->fs = fs;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader *initRouteLoader(QString lib_path)
{
    RouteLoader *loader = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetRouteLoader getRouteLoader = (GetRouteLoader) lib.resolve("getRouteLoader");

        if (getRouteLoader)
        {
            loader = getRouteLoader();
        }
    }

    return loader;
}
