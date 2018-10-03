//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     ROUTE_LOADER_H
#define     ROUTE_LOADER_H

#include    <QObject>

#include    "filesystem.h"

#include    <osg/Geometry>

#if defined(ROUTE_LOADER_LIB)
    #define ROUTE_LOADER_EXPORT Q_DECL_EXPORT
#else
    #define ROUTE_LOADER_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class  ROUTE_LOADER_EXPORT RouteLoader : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit RouteLoader(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~RouteLoader();

    /// Load route
    virtual osg::Node *load(QString route_path) = 0;

    /// Get cartesian position
    virtual osg::Vec3f getPosition(float rail_coord) = 0;

    /// Setting up filesystem object
    void setFileSystem(FileSystem *fs);

protected:

    FileSystem  *fs;
};

/*!
 *
 *
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef RouteLoader* (*GetRouteLoader)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_ROUTE_LOADER(ClassName) \
    extern "C" Q_DECL_EXPORT RouteLoader *getRouteLoader() \
    {\
        return new (ClassName)(); \
    }

extern "C" Q_DECL_EXPORT RouteLoader *initRouteLoader(QString lib_path);

#endif // ROUTE_LOADER_H
