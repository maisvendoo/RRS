//------------------------------------------------------------------------------
//
//      Loader for ZDSimulator routes
//      (c) maisvendoo, 26/11/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Loader for ZDSimulator routes
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 26/11/2018
 */

#ifndef     SCENE_LOADER_H
#define     SCENE_LOADER_H

#include    <string>
#include    <map>
#include    <vector>
#include    <fstream>

#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>

#include    "abstract-loader.h"

#include    "object-data.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SceneLoader : public RouteLoader
{
public:

    /// Constructor for load route
    SceneLoader();

    void load(std::string routeDir);

    osgGA::GUIEventHandler *getCameraEventHandler(int direction, float camera_height);

protected:

    /// List objects from file objects.ref
    std::map<std::string, object_ref_t> objectRef;

    /// List objects from file route1.map
    std::vector<object_map_t> objectMap;

    /// Destructor
    ~SceneLoader() {}

    /// Load data from route files
    ReadResult loadDataFile(const std::string &filepath);

    /// Load data from *.ref file
    ReadResult loadObjectRef(std::istream &stream);

    /// Load data from *.map file
    ReadResult loadObjectMap(std::istream &stream);
};

#endif // SCENE_LOADER_H
