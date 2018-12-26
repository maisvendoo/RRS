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

#include    "scene-loader.h"
#include    "model-loader.h"
#include    "string-funcs.h"

#include    <sstream>

#include    <osg/MatrixTransform>

#include    "filesystem.h"

#include    "route-path.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SceneLoader::SceneLoader() : RouteLoader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SceneLoader::load(std::string routeDir)
{
    FileSystem &fs = FileSystem::getInstance();
    this->routeDir = fs.getNativePath(routeDir);    

    loadDataFile(this->routeDir + fs.separator() + "objects.ref");
    loadDataFile(this->routeDir + fs.separator() + "route1.map");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotionPath *SceneLoader::getMotionPath(int direction)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string track_name = routeDir + fs.separator() +  "route";

    if (direction > 0)
    {
        track_name += "1.trk";
    }
    else
    {
        track_name += "2.trk";
    }

    return new RoutePath(track_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReadResult SceneLoader::loadDataFile(const std::string &filepath)
{
    std::string fileName = osgDB::findDataFile(filepath);

    if (fileName.empty())
        return FILE_NOT_FOUND;

    std::ifstream stream(fileName.c_str(), std::ios::in);

    if (!stream)
        return FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(fileName);

    if (ext == "ref")
    {
        return loadObjectRef(stream);
    }

    if (ext == "map")
    {
        return loadObjectMap(stream);
    }

    return FILE_NOT_HANDLED;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::string> parse_line(const std::string &line)
{
    char delimiter = '\t';
    std::string tmp = line + delimiter;
    std::vector<std::string> tokens;

    size_t pos = 0;

    while ( (pos = tmp.find(delimiter)) != std::string::npos )
    {
        std::string token = tmp.substr(0, pos);
        tmp.erase(0, pos + 1);
        tokens.push_back(token);
    }

    return tokens;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReadResult SceneLoader::loadObjectRef(std::istream &stream)
{
    std::string mode = "";

    while (!stream.eof())
    {
        std::string line;
        std::getline(stream, line);

        if (line.empty())
            continue;

        if (line.at(0) == ';')
            continue;

        if (line.at(0) == '[')
        {
            std::string tmp = delete_symbol(line, '[');
            tmp = delete_symbol(tmp, ']');
            mode = delete_symbol(tmp, '\r');
            continue;
        }

        std::string tmp = delete_symbol(line, '\r');

        object_ref_t object;

        std::vector<std::string> tokens = parse_line(tmp);

        if (tokens.size() < 3)
            continue;

        object.name = tokens[0];
        object.model_path = tokens[1];
        object.texture_path = tokens[2];


        FileSystem &fs = FileSystem::getInstance();        

        object.model_path = routeDir + object.model_path;
        object.texture_path = routeDir + object.texture_path;
        object.model_path = fs.toNativeSeparators(object.model_path);
        object.texture_path = fs.toNativeSeparators(object.texture_path);

        model_info_t model_info;
        model_info.name = object.name;
        model_info.filepath = object.model_path;
        model_info.texture_path = object.texture_path;

        if (mode == "mipmap")
            model_info.mipmap = true;
        else
            model_info.mipmap = false;

        object.model_node = createLODNode(model_info);        

        objectRef.insert(std::pair<std::string, object_ref_t>(object.name, object));
    }

    return FILE_READ_SUCCESS;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReadResult SceneLoader::loadObjectMap(std::istream &stream)
{
    std::string prev_name = "";    

    while (!stream.eof())
    {
        std::string line = "";
        std::getline(stream, line);

        if (line.empty())
            continue;

        if (line.at(0) == ';')
            continue;

        if (line.at(0) == ',')
            line = prev_name + line;

        std::string tmp = delete_symbol(line, '\r');
        tmp = delete_symbol(tmp, ';');
        std::replace(tmp.begin(), tmp.end(), ',', ' ');

        std::istringstream ss(tmp);

        object_map_t object;

        ss >> object.name
           >> object.position.x() >> object.position.y() >> object.position.z()
           >> object.attitude.x() >> object.attitude.y() >> object.attitude.z();

        std::getline(stream, line);

        prev_name = object.name;

        object.caption = delete_symbol(line, '\r');

        object.attitude.x() *= osg::PIf / 180.0f;
        object.attitude.y() *= osg::PIf / 180.0f;
        object.attitude.z() *= osg::PIf / 180.0f;

        if (objectRef[object.name].model_node.valid())
        {
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

            osg::Matrixf m1 = osg::Matrixf::translate(object.position);
            osg::Matrixf m2 = osg::Matrixf::rotate(-object.attitude.z(),
                                                   osg::Vec3(0, 0, 1));

            osg::Matrixf m3 = osg::Matrixf::rotate(-object.attitude.x(),
                                                   osg::Vec3(1, 0, 0));

            osg::Matrixf m4 = osg::Matrixf::rotate(-object.attitude.y(),
                                                   osg::Vec3(0, 1, 0));

            transform->setMatrix(m3 * m2 * m4 * m1);

            transform->addChild(objectRef[object.name].model_node.get());
            root->addChild(transform.get());
        }
    }

    return FILE_READ_SUCCESS;
}

GET_ROUTE_LOADER(SceneLoader)
