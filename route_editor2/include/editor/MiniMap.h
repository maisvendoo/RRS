//------------------------------------------------------------------------------
//
//      Мини-карта редактора (по мотивам TSRE5 MapWindow/MapDataOSM):
//      подложка OSM/спутник (тайлы, кэш на диск, фоновая загрузка),
//      траектории маршрута, объекты, камера; клик - телепорт камеры
//
//------------------------------------------------------------------------------

#ifndef EDITOR_MINIMAP_H
#define EDITOR_MINIMAP_H

#include "EditorContext.h"

#include <vsgImGui/Texture.h>

#include <QImage>
#include <QMutex>

#include <map>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MiniMap
{
public:

    explicit MiniMap(EditorContext& context);
    ~MiniMap();

    /// Рисовать окно "Карта" (вызывается из EditorGui каждый кадр)
    void draw();

    /// Гео-якорь маршрута (центр bbox импорта; читается из description.xml)
    void setRouteAnchor(double lat, double lon);

private:

    struct TileKey
    {
        int z = 0;
        int x = 0;
        int y = 0;

        bool operator<(const TileKey& other) const
        {
            if (z != other.z) return z < other.z;
            if (x != other.x) return x < other.x;
            return y < other.y;
        }
    };

    struct Tile
    {
        // Данные тайла (копия RGBA в vsg::Data после загрузки)
        vsg::ref_ptr<vsg::Data> data;
        vsg::ref_ptr<vsgImGui::Texture> texture;
        bool requested = false;
    };

    EditorContext& context_;

    double route_lat_ = 55.75;      // якорь маршрута (WGS84 локального нуля)
    double route_lon_ = 37.6173;

    int zoom_ = 14;
    int map_style_ = 0;             // 0 - OSM, 1 - спутник ArcGIS

    // Текущие видимые тайлы
    std::map<TileKey, Tile> tiles_;

    // Очередь готовых изображений от загрузчика (защищена мьютексом)
    QMutex ready_mutex_;
    std::map<TileKey, QImage> ready_images_;

    // Поток загрузки (реализация в cpp: QNetworkAccessManager + loop)
    struct Loader;
    Loader* loader_ = nullptr;

    // Инициализация потока (лениво)
    void ensure_loader();

    void request_tile(const TileKey& key);
    vsg::ref_ptr<vsgImGui::Texture> texture_for(const TileKey& key);

    // Проекции
    void world_to_latlon(double wx, double wy, double& lat, double& lon) const;
    void latlon_to_tile(double lat, double lon, double& tx, double& ty) const;
    static void latlon_to_world(double lat, double lon, double lat0, double lon0,
                                double& wx, double& wy);

    QString tile_url(const TileKey& key) const;
    QString tile_cache_path(const TileKey& key) const;
};

#endif // EDITOR_MINIMAP_H
