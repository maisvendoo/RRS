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

#ifndef     ROUTE_LOADING_HANDLE_H
#define     ROUTE_LOADING_HANDLE_H

#include    <osgGA/GUIEventHandler>
#include    <osgViewer/Viewer>

#include    "route-info.h"

/*!
 * \file
 * \brief Route loading handler
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteLoadingHandle : public osgGA::GUIEventHandler
{
public:

    /// Constructor
    RouteLoadingHandle();

    /// Destructor
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

private:    

    /// Route static scene root node
    osg::ref_ptr<osg::Group>    routeRoot;

    /// Route loading flag
    bool                        is_loaded;

    /// Current camera handler
    osgGA::GUIEventHandler      *cameraHandler;

    /// Current route ID
    unsigned int                current_route_id;

    /// Routes list info
    std::map<unsigned int, route_info_t> routes_info;


    /// Load routes info list
    bool loadRoutesInfo(std::string path);

    /// Load route form directory
    bool loadRoute(const std::string &routeDir, osgViewer::Viewer *viewer);

    /// Load route by ID
    bool loadRouteByID(unsigned int id, osgViewer::Viewer *viewer);

    /// Init common graphical engine settings
    bool initEngineSettings(osg::Group *routeRoot);
};

#endif // ROUTE_LOADING_HANDLE_H
