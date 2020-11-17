#include <QDir>
#include <QDirIterator>
#include "filesystem.h"
#include "CfgReader.h"
#include "models/TrainsModel.h"

TrainsModel::TrainsModel(QObject *parent)
    : QAbstractListModel (parent)
    , m_visibleTrainsCount(0)
{
    initModel();
    loadTrainsList();
    refresh();
}

int TrainsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleTrainsCount;
}

QVariant TrainsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();

    if(row < 0 || row >= m_trainInfoList.count())
        return QVariant();

    switch(role) {
    case TrainsName: return m_trainInfoList.at(row).name;
    case TrainsDescription: return m_trainInfoList.at(row).description;
    case TrainsDir: return m_trainInfoList.at(row).dir;
    case TrainsImage: return m_trainInfoList.at(row).image;
    }
    return QVariant();
}

QHash<int, QByteArray> TrainsModel::roleNames() const
{
    return m_roleNames;
}

bool TrainsModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleTrainsCount < m_trainInfoList.size();
}

void TrainsModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    int remainder = m_trainInfoList.count() - m_visibleTrainsCount;
    int itemsToFetch = qMin(20, remainder);

    if (itemsToFetch <= 0)
        return;

    beginInsertRows(QModelIndex(), m_visibleTrainsCount, m_visibleTrainsCount + itemsToFetch - 1);
    m_visibleTrainsCount += itemsToFetch;
    endInsertRows();
}

void TrainsModel::refresh()
{
    beginResetModel();
    m_visibleTrainsCount = 0;
    endResetModel();
}

void TrainsModel::initModel()
{
    m_roleNames[RoleNames::TrainsName] = "name";
    m_roleNames[RoleNames::TrainsDescription] = "description";
    m_roleNames[RoleNames::TrainsDir] = "dir";
    m_roleNames[RoleNames::TrainsImage] = "image";
}

void TrainsModel::loadTrainsList()
{
    const FileSystem &fs = FileSystem::getInstance();
    const QDir routes(QString(fs.getTrainsDir().c_str()));
    QDirIterator trains_dirs(routes.path(), QStringList() << "*.xml", QDir::NoDotAndDotDot | QDir::Files);

    while (trains_dirs.hasNext())
    {
        TrainsInfo info;
        QString fullPath = trains_dirs.next();
        QFileInfo fileInfo(fullPath);
        info.dir = fileInfo.baseName();

        CfgReader cfg;
        if (cfg.load(fullPath))
        {
            QString secName = "Common";

            cfg.getString(secName, "Title", info.name);
            cfg.getString(secName, "Description", info.description);

            const QString image = info.dir + QDir::separator() + "shotcut.png";
            if(QFile::exists(image))
                info.image = QString("file:/" + image);

            m_trainInfoList.append(info);
        }
    }
}
