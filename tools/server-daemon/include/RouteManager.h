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

#ifndef     ROUTEMANAGER_H
#define     ROUTEMANAGER_H

#include    <QObject>
#include    <QString>
#include    <QVector>
#include    <QDir>
#include    <QFile>
#include    "CfgReader.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct RouteInfo
{
    QString     name;           // Имя папки маршрута
    QString     title;          // Заголовок из XML
    QString     description;    // Описание из XML
    QString     author;         // Автор из XML
    QString     version;        // Версия из XML
    QString     road;           // Дорога (СКЖД и т.д.)
    QString     routePath;      // Полный путь к маршруту
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class RouteManager : public QObject
{
    Q_OBJECT

public:

    explicit RouteManager(QObject* parent = nullptr);

    bool loadRoutes(const QString& routesPath);
    QVector<RouteInfo> getRoutes() const { return m_routes; }
    RouteInfo getRouteByName(const QString& name) const;
    QString getRoutePath(const QString& name) const;
    int getRoutesCount() const { return m_routes.size(); }
    void clear() { m_routes.clear(); }

private:

    bool parseRouteDescription(const QString& filePath, RouteInfo& info);

    QVector<RouteInfo>  m_routes;
    QString             m_routesPath;
};

#endif // ROUTEMANAGER_H