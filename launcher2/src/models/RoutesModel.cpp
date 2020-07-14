#include "models/RoutesModel.h"
#include "filesystem.h"
#include "CfgReader.h"
#include <QDirIterator>
#include <QDir>

RoutesModel::RoutesModel(QObject *parent)
    : QAbstractListModel (parent)
    , m_visibleRoutesCount(0)
{
    initModel();
    loadRoutesList();
    refresh();
}

int RoutesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleRoutesCount;
}

QVariant RoutesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();

    if(row < 0 || row >= m_routeInfoList.count())
        return QVariant();

    switch(role) {
    case RouteName: return m_routeInfoList.at(row).name;
    case RouteDescription: return m_routeInfoList.at(row).description;
    case RouteDir: return m_routeInfoList.at(row).dir;
    case RouteImage: return m_routeInfoList.at(row).image;
    }
    return QVariant();
}

QHash<int, QByteArray> RoutesModel::roleNames() const
{
    return m_roleNames;
}

bool RoutesModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleRoutesCount < m_routeInfoList.size();
}

void RoutesModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    int remainder = m_routeInfoList.count() - m_visibleRoutesCount;
    int itemsToFetch = qMin(20, remainder);

    if (itemsToFetch <= 0)
        return;

    beginInsertRows(QModelIndex(), m_visibleRoutesCount, m_visibleRoutesCount + itemsToFetch - 1);
    m_visibleRoutesCount += itemsToFetch;
    endInsertRows();
}

void RoutesModel::refresh()
{
    beginResetModel();
    m_visibleRoutesCount = 0;
    endResetModel();
}

void RoutesModel::initModel()
{
    m_roleNames[RoleNames::RouteName] = "name";
    m_roleNames[RoleNames::RouteDescription] = "description";
    m_roleNames[RoleNames::RouteDir] = "dir";
    m_roleNames[RoleNames::RouteImage] = "image";
}

void RoutesModel::loadRoutesList()
{
    const FileSystem &fs = FileSystem::getInstance();
    const QDir routes(QString(fs.getRouteRootDir().c_str()));
    QDirIterator route_dirs(routes.path(), QStringList(), QDir::NoDotAndDotDot | QDir::Dirs);

    while (route_dirs.hasNext())
    {
        RouteInfo info;
        info.dir = route_dirs.next();

        CfgReader cfg;
        if (cfg.load(info.dir + QDir::separator() + "description.xml"))
        {
            QString secName = "Route";

            cfg.getString(secName, "Title", info.name);
            cfg.getString(secName, "Description", info.description);

            const QString image = info.dir + QDir::separator() + "shotcut.png";
            if(QFile::exists(image))
                info.image = QString("file:/" + image);

            m_routeInfoList.append(info);
        }

    }
}
