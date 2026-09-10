//------------------------------------------------------------------------------
//
//      osm-import: высоты из SRTM .hgt
//
//------------------------------------------------------------------------------

#include "geo-height.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <cmath>
#include <cstring>
#include <sstream>

namespace osm
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GeoHeight::GeoHeight(const std::string& path)
{
    const QFileInfo info(QString::fromStdString(path));

    if (info.isDir())
    {
        const QDir dir(info.absoluteFilePath());
        const QStringList files = dir.entryList(QStringList() << "*.hgt" << "*.HGT",
                                                 QDir::Files);

        for (const QString& name : files)
        {
            load_tile(dir.filePath(name).toStdString());
        }
    }
    else if (info.isFile())
    {
        load_tile(path);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool GeoHeight::valid() const
{
    return hasData();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GeoHeight::load_tile(const std::string& file)
{
    QFile f(QString::fromStdString(file));

    if (!f.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QByteArray raw = f.readAll();
    f.close();

    // Размер сетки из объёма файла: 3601^2*2 (SRTM-1) или 1201^2*2 (SRTM-3)
    const std::size_t count = static_cast<std::size_t>(raw.size()) / 2;

    int size = 0;

    if (count == 3601 * 3601) size = 3601;
    else if (count == 1201 * 1201) size = 1201;
    else
    {
        return;
    }

    // Имя файла: [NS]yy[EWWW].hgt -> юго-западный угол тайла
    const QString base = QFileInfo(QString::fromStdString(file)).baseName();

    bool ok_south = false;
    bool ok_west = false;
    int lat_floor = 0;
    int lon_floor = 0;

    const QString body = base;

    const int ns_sign = (body[0] == 'S' || body[0] == 's') ? -1 : 1;
    const int ew_sign = (body[3] == 'W' || body[3] == 'w') ? -1 : 1;

    lat_floor = ns_sign * body.mid(1, 2).toInt(&ok_south);
    lon_floor = ew_sign * body.mid(4, 3).toInt(&ok_west);

    if (!ok_south || !ok_west)
    {
        return;
    }

    Tile tile;
    tile.lat_floor = lat_floor;
    tile.lon_floor = lon_floor;
    tile.size = size;
    tile.data.resize(count);

    // Big-endian int16
    for (std::size_t i = 0; i < count; ++i)
    {
        const unsigned char b0 = static_cast<unsigned char>(raw[2 * i]);
        const unsigned char b1 = static_cast<unsigned char>(raw[2 * i + 1]);
        tile.data[i] = static_cast<std::int16_t>((b0 << 8) | b1);
    }

    const std::uint64_t key =
            (static_cast<std::uint64_t>(static_cast<std::uint32_t>(lat_floor)) << 32) |
            static_cast<std::uint32_t>(lon_floor);

    tiles_[key] = std::move(tile);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double GeoHeight::height(double lat, double lon) const
{
    const int lat_floor = static_cast<int>(std::floor(lat));
    const int lon_floor = static_cast<int>(std::floor(lon));

    const std::uint64_t key =
            (static_cast<std::uint64_t>(static_cast<std::uint32_t>(lat_floor)) << 32) |
            static_cast<std::uint32_t>(lon_floor);

    const auto it = tiles_.find(key);

    if (it == tiles_.end())
    {
        return 0.0;
    }

    const Tile& tile = it->second;

    const double fy = (lat - lat_floor) * (tile.size - 1);
    const double fx = (lon - lon_floor) * (tile.size - 1);

    // Строки идут с севера на юг
    const int row = static_cast<int>(std::round((tile.size - 1) - fy));
    const int col = static_cast<int>(std::round(fx));

    const auto at = [&](int r, int c) -> double
    {
        r = std::max(0, std::min(tile.size - 1, r));
        c = std::max(0, std::min(tile.size - 1, c));
        return static_cast<double>(tile.data[r * tile.size + c]);
    };

    // Двулинейная интерполяция 4 соседей
    const int r0 = static_cast<int>(std::floor((tile.size - 1) - fy));
    const int c0 = static_cast<int>(std::floor(fx));
    const double ty = ((tile.size - 1) - fy) - r0;
    const double tx = fx - c0;

    (void)row;
    (void)col;

    const double h00 = at(r0, c0);
    const double h10 = at(r0 + 1, c0);
    const double h01 = at(r0, c0 + 1);
    const double h11 = at(r0 + 1, c0 + 1);

    const double h0 = h00 * (1.0 - ty) + h10 * ty;
    const double h1 = h01 * (1.0 - ty) + h11 * ty;

    return h0 * (1.0 - tx) + h1 * tx;
}

} // namespace osm
