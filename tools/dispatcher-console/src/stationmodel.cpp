#include    <stationmodel.h>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StationModel::StationModel()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationModel::clear()
{
    m_stations.clear();
    m_stations_order.clear();
    m_block_sections.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationModel::loadStations(const QVector<station_info_t> &stations)
{
    m_stations = stations;

    if (m_stations.isEmpty())
    {
        m_stations_order.clear();
        m_block_sections.clear();
        return;
    }

    // Сортируем станции вдоль оси Y (главный ход линии - по оси Y)
    std::sort(m_stations.begin(), m_stations.end(),
              [](const station_info_t &a, const station_info_t &b)
              {
                  if (qAbs(a.pos_y - b.pos_y) < 1.0)
                      return a.pos_x < b.pos_x;
                  return a.pos_y < b.pos_y;
              });

    m_stations_order.clear();
    for (const auto &s : m_stations)
        m_stations_order.append(s.name);

    rebuildBlockSections();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationModel::setStationsOrder(const QStringList &ordered_names)
{
    m_stations_order = ordered_names;
    rebuildBlockSections();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QVector<station_info_t> &StationModel::stations() const
{
    return m_stations;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QStringList &StationModel::stationsOrder() const
{
    return m_stations_order;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QVector<block_section_t> &StationModel::blockSections() const
{
    return m_block_sections;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StationModel::isEmpty() const
{
    return m_stations.isEmpty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationModel::rebuildBlockSections()
{
    m_block_sections.clear();

    if (m_stations_order.size() < 2)
        return;

    for (int i = 0; i < m_stations_order.size() - 1; ++i)
    {
        block_section_t bs;
        bs.station_a = m_stations_order[i];
        bs.station_b = m_stations_order[i + 1];
        m_block_sections.append(bs);
    }
}