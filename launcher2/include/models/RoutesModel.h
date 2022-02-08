#ifndef ROUTESMODEL_H
#define ROUTESMODEL_H
#include <QAbstractListModel>

struct RouteInfo
{
    QString dir;
    QString name;
    QString image;
    QString description;
};

class RoutesModel : public QAbstractListModel
{
    Q_GADGET

    /**
     * @brief Роли.
     * @param RouteName Имя маршрута
     * @param RouteDescription Описание маршрута
     * @param RouteDir Директория с маршрутом
     */
    enum RoleNames {
        RouteName,
        RouteDescription,
        RouteImage,
        RouteDir
    };

public:
    RoutesModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    QHash<int, QByteArray> m_roleNames;
    QList<RouteInfo>       m_routeInfoList;
    int                    m_visibleRoutesCount;

    void refresh();
    void initModel();
    void loadRoutesList();
};

#endif // ROUTESMODEL_H
