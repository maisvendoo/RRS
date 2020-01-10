#include "models/WaypointModel.h"
#include <QTextStream>
#include <QFile>
#include <QDir>

WaypointModel::WaypointModel(QObject *parent)
    : QAbstractListModel (parent)
{
    initModel();
    loadWaypointList();
    refresh();
}

int WaypointModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_waypointInfoList.count();
}

QVariant WaypointModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();

    if(row < 0 || row >= m_waypointInfoList.count())
        return QVariant();

    switch(role) {
    case Name: return m_waypointInfoList.at(row).name;
    case Forward: return m_waypointInfoList.at(row).forward_coord;
    case Backward: return m_waypointInfoList.at(row).backward_coord;
    }
    return QVariant();
}

void WaypointModel::setRoute(const QString &routeDir)
{
    m_routeDir = routeDir;
    refresh();
}

QHash<int, QByteArray> WaypointModel::roleNames() const
{
    return m_roleNames;
}

void WaypointModel::refresh()
{
    beginResetModel();
    m_waypointInfoList.clear();
    loadWaypointList();
    endResetModel();
}

void WaypointModel::initModel()
{
    m_roleNames[RoleNames::Name] = "name";
    m_roleNames[RoleNames::Forward] = "forward";
    m_roleNames[RoleNames::Backward] = "backward";
}

void WaypointModel::loadWaypointList()
{
    QFile file(m_routeDir + QDir::separator() + "waypoints.conf");
    if (!file.open(QIODevice::ReadOnly))
        return;

    while (!file.atEnd())
    {
        QString line = file.readLine();
        QTextStream ss(&line);
        WaypointInfo info;
        ss >> info.name >> info.forward_coord >> info.backward_coord;

        m_waypointInfoList.push_back(info);
    }
}
