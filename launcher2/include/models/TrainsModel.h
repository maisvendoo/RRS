#ifndef TRAINSMODEL_H
#define TRAINSMODEL_H
#include <QAbstractListModel>

struct TrainsInfo
{
    QString dir;
    QString name;
    QString image;
    QString description;
};

class TrainsModel : public QAbstractListModel
{
    Q_GADGET

    /**
     * @brief Роли.
     * @param TrainsName Название ПС
     * @param TrainsDescription Описание ПС
     * @param TrainsDir Директория с ПС
     */
    enum RoleNames {
        TrainsName,
        TrainsDescription,
        TrainsImage,
        TrainsDir
    };
public:
    TrainsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    QHash<int, QByteArray> m_roleNames;
    QList<TrainsInfo>      m_trainInfoList;
    int                    m_visibleTrainsCount;

    void refresh();
    void initModel();
    void loadTrainsList();
};

#endif // TRAINSMODEL_H
