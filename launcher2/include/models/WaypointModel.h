#ifndef WAYPOINTMODEL_H
#define WAYPOINTMODEL_H
#include <QAbstractListModel>

struct WaypointInfo
{
    QString name;
    double  forward_coord;
    double  backward_coord;
};

class WaypointModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief Роли.
     * @param Name Наимениование станции
     * @param Forward Направление
     * @param Backward Направление
     */
    enum RoleNames {
        Name,
        Forward,
        Backward
    };
public:
    WaypointModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Q_INVOKABLE void setRoute(const QString& routeDir);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QHash<int, QByteArray> m_roleNames;
    QList<WaypointInfo>    m_waypointInfoList;
    QString                m_routeDir;

    void refresh();
    void initModel();
    void loadWaypointList();
};

#endif // WAYPOINTMODEL_H
