//------------------------------------------------------------------------------
//
//      Route loading handler
//      (c) maisvendoo, 11/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Route loading handler
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 11/12/2018
 */

#include    "route-loading-handle.h"

#include    "filesystem.h"
#include    "config-reader.h"
#include    "get-value.h"
#include    "abstract-loader.h"
#include    "trajectory-element.h"

#include    <osg/BlendFunc>
#include    <osg/CullFace>

#include    "lighting.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoadingHandle::RouteLoadingHandle()
    : routeRoot(new osg::Group)
    , is_loaded(false)
    , cameraHandler(nullptr)
    , current_route_id(0)
{
    FileSystem &fs = FileSystem::getInstance();

    // Read aviable routes list
    loadRoutesInfo(fs.getConfigDir() + fs.separator() + "routes-list.xml");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoadingHandle::handle(const osgGA::GUIEventAdapter &ea,
                                osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            break;
        }

    case osgGA::GUIEventAdapter::USER:
        {
            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            const traj_element_t *te = dynamic_cast<const traj_element_t *>(ea.getUserData());
            current_route_id = te->route_id;

            if (current_route_id != 0)
            {
                if (!is_loaded)
                {
                    if (!loadRouteByID(te->route_id, viewer))
                        break;

                    is_loaded = initEngineSettings(routeRoot.get());
                }
            }
            else
            {
                is_loaded = false;

                if (routeRoot != nullptr)
                {
                    // Clear scene
                    unsigned int n = routeRoot->getNumChildren();
                    routeRoot->removeChildren(0, n);
                }

                // Clear camera motion handler
                if (cameraHandler != nullptr)
                    viewer->removeEventHandler(cameraHandler);

                viewer->realize();
            }

            break;
        }

    default:

        break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoadingHandle::loadRoutesInfo(std::string path)
{
    if (path.empty())
        return false;

    ConfigReader cfg(path);

    if (!cfg.isOpenned())
        return false;

    osgDB::XmlNode *config_node = cfg.getConfigNode();

    if (config_node == nullptr)
        return false;

    for (auto it = config_node->children.begin(); it != config_node->children.end(); ++it)
    {
        route_info_t route_info;

        osgDB::XmlNode *id = cfg.findSection(*it, "id");
        getValue(id->contents, route_info.id);

        osgDB::XmlNode *name = cfg.findSection(*it, "Name");
        getValue(name->contents, route_info.name);

        if (route_info.id != 0)
        {
            routes_info.insert(std::pair<unsigned int,
                               route_info_t>(route_info.id, route_info));
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoadingHandle::loadRoute(const std::string &routeDir,
                                   osgViewer::Viewer *viewer)
{
    if (routeDir.empty())
        return false;

    FileSystem &fs = FileSystem::getInstance();
    std::string routeType = osgDB::findDataFile(routeDir + fs.separator() + "route-type");

    if (routeType.empty())
        return false;

    std::ifstream stream(routeType);

    if (!stream.is_open())
        return false;

    std::string routeExt = "";
    stream >> routeExt;

    if (routeExt.empty())
        return false;

    std::string routeLoaderPlugin = routeExt + "-route-loader";

    osg::ref_ptr<RouteLoader> loader = loadRouteLoader("../plugins", routeLoaderPlugin);

    if (!loader.valid())
        return false;

    loader->load(routeDir);
    routeRoot = loader->getRoot();

    cameraHandler = loader->getCameraEventHandler(1, 3.0f);
    viewer->addEventHandler(cameraHandler);
    viewer->setSceneData(routeRoot.get());

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoadingHandle::loadRouteByID(unsigned int id, osgViewer::Viewer *viewer)
{
    FileSystem &fs = FileSystem::getInstance();
    route_info_t route_info = routes_info[id];
    std::string routePath = fs.getRouteRootDir() + fs.separator() + route_info.name;

    if (!loadRoute(routePath, viewer))
        return false;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoadingHandle::initEngineSettings(osg::Group *routeRoot)
{
    if (routeRoot == nullptr)
        return false;

    // Common graphics settings
    osg::StateSet *stateset = routeRoot->getOrCreateStateSet();
    osg::BlendFunc *blendFunc = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    stateset->setAttributeAndModes(blendFunc);
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateset->setMode(GL_ALPHA, osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    osg::ref_ptr<osg::CullFace> cull = new osg::CullFace;
    cull->setMode(osg::CullFace::BACK);
    stateset->setAttributeAndModes(cull.get(), osg::StateAttribute::ON);

    // Set lighting
    initEnvironmentLight(routeRoot,
                         osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f),
                         1.0f,
                         0.0f,
                         90.0f);

    return true;
}
