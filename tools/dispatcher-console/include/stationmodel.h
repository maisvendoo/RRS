#ifndef STATIONMODEL_H
#define STATIONMODEL_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QPointF>

struct station_info_t
{
    QString name;
    double pos_x = 0.0;
    double pos_y = 0.0;
};

struct block_section_t
{
    QString station_a;
    QString station_b;
    double length = 0.0;
};

class StationModel
{
public:
    StationModel();

    void clear();

    void loadStations(const QVector<station_info_t> &stations);

    void setStationsOrder(const QStringList &ordered_names);

    const QVector<station_info_t> &stations() const;

    const QStringList &stationsOrder() const;

    const QVector<block_section_t> &blockSections() const;

    bool isEmpty() const;

private:
    QVector<station_info_t> m_stations;
    QStringList m_stations_order;
    QVector<block_section_t> m_block_sections;

    void rebuildBlockSections();
};

#endif