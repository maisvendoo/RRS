#ifndef ROUTE_LOADER_H
#define ROUTE_LOADER_H

#include "MotionPath.h"
#include <string>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

enum ReadResult
{
    FILE_READ_SUCCESS,
    FILE_NOT_FOUND,
    FILE_NOT_HANDLED
};

class RouteLoader
{
public:
    RouteLoader();

    virtual void load(const std::string& routeDir, float view_dist = 1000.0f) = 0;

    virtual vsg::Group* getRoot();

    virtual MotionPath* getMotionPath(int direction) = 0;

protected:
    std::string routeDir;
    vsg::ref_ptr<vsg::Group> root;
    virtual ~RouteLoader();
    virtual ReadResult loadDataFile(const std::string& filepath) = 0;
};

using GetRouteLoader = RouteLoader* (*)();

#define GET_ROUTE_LOADER(ClassName) \
    extern "C" RouteLoader* getRouteLoader() \
    { \
        return new (ClassName)(); \
    }

extern "C" RouteLoader* loadRouteLoader(const std::string& path, const std::string& name);

#endif // ROUTE_LOADER_H

/* objects.ref 1
    label path_to.dmd path_to.tga
*/

/* objects.ref 2
    label path_to.gltf
*/

/* Route

*/
