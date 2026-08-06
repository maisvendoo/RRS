//------------------------------------------------------------------------------
//
//  Route manager
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Route manager
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    "RouteManager.h"
#include    <QDebug>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RouteManager::RouteManager(QObject* parent) : QObject(parent)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool RouteManager::loadRoutes(const QString& routesPath)
{
    m_routes.clear();
    m_routesPath = routesPath;

    QDir routesDir(routesPath);
    if (!routesDir.exists())
    {
        qWarning() << "Routes directory does not exist:" << routesPath;
        return false;
    }

    QStringList routeDirs = routesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& dirName : routeDirs)
    {
        QString routePath = routesDir.absolutePath() + "/" + dirName;
        QString descriptionFile = routePath + "/description.xml";

        if (!QFile::exists(descriptionFile))
        {
            qWarning() << "No description.xml in route:" << dirName;
            continue;
        }

        RouteInfo info;
        info.name = dirName;
        info.routePath = routePath;

        if (parseRouteDescription(descriptionFile, info))
        {
            m_routes.append(info);
            qInfo() << "Loaded route:" << info.title
                    << "(version:" << info.version << ")";
        }
    }

    return !m_routes.isEmpty();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool RouteManager::parseRouteDescription(const QString& filePath, RouteInfo& info)
{
    CfgReader cfg;

    if (!cfg.load(filePath))
    {
        qWarning() << "Failed to load description file:" << filePath;
        return false;
    }

    QDomNode routeNode = cfg.getFirstSection("Route");
    if (routeNode.isNull())
    {
        qWarning() << "No Route section in:" << filePath;
        return false;
    }

    QString title;
    if (cfg.getString(routeNode, "Title", title))
    {
        info.title = title;
    }
    else
    {
        info.title = info.name;
    }

    QString description;
    if (cfg.getString(routeNode, "Description", description))
    {
        info.description = description;
    }

    QString author;
    if (cfg.getString(routeNode, "Authors", author))
    {
        info.author = author;
    }

    QString version;
    if (cfg.getString(routeNode, "Version", version))
    {
        info.version = version;
    }

    QString road;
    if (cfg.getString(routeNode, "Road", road))
    {
        info.road = road;
    }

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RouteInfo RouteManager::getRouteByName(const QString& name) const
{
    for (const RouteInfo& route : m_routes)
    {
        if (route.name == name || route.title == name)
        {
            return route;
        }
    }
    return RouteInfo();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QString RouteManager::getRoutePath(const QString& name) const
{
    RouteInfo info = getRouteByName(name);
    if (info.name.isEmpty())
    {
        return QString();
    }
    return m_routesPath + "/" + info.name;
}