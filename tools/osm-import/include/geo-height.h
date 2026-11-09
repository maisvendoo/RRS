//------------------------------------------------------------------------------
//
//      osm-import: высоты из SRTM .hgt (GeoHgtFile по мотивам TSRE5)
//
//------------------------------------------------------------------------------

#ifndef GEO_HEIGHT_H
#define GEO_HEIGHT_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace osm
{

//------------------------------------------------------------------------------
/// Читатель SRTM .hgt: сетка 1x1 градус, SRTM-1 (3601x3601) или
/// SRTM-3 (1201x1201), int16 big-endian, высоты в метрах
//------------------------------------------------------------------------------
class GeoHeight
{
public:

    /// path - файл NxxEyyy.hgt или каталог с такими файлами
    explicit GeoHeight(const std::string& path);

    bool valid() const;

    /// Высота (м) по широте/долготе; 0.0 если данных нет
    double height(double lat, double lon) const;

    /// Заполнено ли хоть что-то
    bool hasData() const { return !tiles_.empty(); }

private:

    struct Tile
    {
        int lat_floor = 0;
        int lon_floor = 0;
        int size = 0;               // 3601 или 1201
        std::vector<int16_t> data;
    };

    void load_tile(const std::string& file);

    mutable std::map<std::uint64_t, Tile> tiles_;
};

} // namespace osm

#endif // GEO_HEIGHT_H
