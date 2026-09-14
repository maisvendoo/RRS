//------------------------------------------------------------------------------
//
//      Мини-карта редактора: подложка OSM/спутник + схема маршрута
//
//------------------------------------------------------------------------------

#include "editor/MiniMap.h"

#include "editor/Camera.h"
#include "editor/Route.h"

#include <topology.h>
#include <trajectory.h>

#include <vsgImGui/imgui.h>
#include <vsgImGui/RenderImGui.h>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------------
// Загрузчик тайлов: отдельный поток с собственным event loop
//------------------------------------------------------------------------------
struct MiniMap::Loader
{
    QThread thread;
    QNetworkAccessManager* manager = nullptr;
    QEventLoop* loop = nullptr;
    MiniMap* owner = nullptr;

};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MiniMap::MiniMap(EditorContext& context)
    : context_(context)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MiniMap::~MiniMap()
{
    if (loader_ != nullptr)
    {
        loader_->thread.quit();
        loader_->thread.wait(3000);
        delete loader_;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::setRouteAnchor(double lat, double lon)
{
    route_lat_ = lat;
    route_lon_ = lon;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MiniMap::tile_url(const TileKey& key) const
{
    if (map_style_ == 1)
    {
        return QString(
            "https://server.arcgisonline.com/ArcGIS/rest/services/"
            "World_Imagery/MapServer/tile/%1/%2/%3")
                .arg(key.z).arg(key.y).arg(key.x);
    }

    return QString("https://tile.openstreetmap.org/%1/%2/%3.png")
                .arg(key.z).arg(key.x).arg(key.y);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MiniMap::tile_cache_path(const TileKey& key) const
{
    const QString style = (map_style_ == 1) ? "sat" : "osm";
    return QString("D:/rrs/caches/map_tiles/%1/%2/%3_%4.png")
                .arg(style).arg(key.z).arg(key.x).arg(key.y);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::latlon_to_tile(double lat, double lon, double& tx, double& ty) const
{
    const double n = std::pow(2.0, zoom_);
    tx = (lon + 180.0) / 360.0 * n;

    const double lat_rad = lat * 0.017453292519943295;
    ty = (1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / M_PI) / 2.0 * n;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::world_to_latlon(double wx, double wy, double& lat, double& lon) const
{
    const double m_lat = 111132.0;
    const double m_lon = 111320.0 * std::cos(route_lat_ * 0.017453292519943295);
    lat = route_lat_ + wy / m_lat;
    lon = route_lon_ + wx / m_lon;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::latlon_to_world(double lat, double lon, double lat0, double lon0,
                              double& wx, double& wy)
{
    const double m_lat = 111132.0;
    const double m_lon = 111320.0 * std::cos(lat0 * 0.017453292519943295);
    wx = (lon - lon0) * m_lon;
    wy = (lat - lat0) * m_lat;
}

//------------------------------------------------------------------------------
// Загрузка тайла в потоке: кэш -> сеть -> готово
//------------------------------------------------------------------------------
void MiniMap::request_tile(const TileKey& key)
{
    // Кэш на диске?
    const QString cache = tile_cache_path(key);
    QFile file(cache);

    if (file.open(QIODevice::ReadOnly))
    {
        const QByteArray raw = file.readAll();
        file.close();

        QImage image;
        if (image.loadFromData(raw))
        {
            QMutexLocker lock(&ready_mutex_);
            ready_images_[key] = image;
        }
        return;
    }

    // В сеть (в потоке загрузчика)
    ensure_loader();

    // Прямой (блокирующий поток) запрос: выполняется В ПОТОКЕ loader
    QMetaObject::invokeMethod(loader_->thread.thread(), [this, key, cache]()
    {
        QNetworkAccessManager manager;
        QEventLoop loop;

        QNetworkRequest request(QUrl(tile_url(key)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "RRS-editor/1.0");

        QNetworkReply* reply = manager.get(request);

        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() == QNetworkReply::NoError)
        {
            const QByteArray data = reply->readAll();

            QImage image;
            if (image.loadFromData(data))
            {
                QDir().mkpath(QFileInfo(cache).absolutePath());

                QFile out(cache);
                if (out.open(QIODevice::WriteOnly))
                {
                    out.write(data);
                    out.close();
                }

                QMutexLocker lock(&ready_mutex_);
                ready_images_[key] = image;
            }
        }

        reply->deleteLater();
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsgImGui::Texture> MiniMap::texture_for(const TileKey& key)
{
    // Есть готовое изображение? Конвертируем в vsg::Data (RGBA)
    QImage image;

    {
        QMutexLocker lock(&ready_mutex_);
        const auto it = ready_images_.find(key);
        if (it == ready_images_.end())
        {
            return nullptr;
        }
        image = it->second;
        ready_images_.erase(it);
    }

    if (image.format() != QImage::Format_RGBA8888)
    {
        image = image.convertToFormat(QImage::Format_RGBA8888);
    }

    image = image.mirrored();

    const int w = image.width();
    const int h = image.height();

    const auto data = vsg::vec4Array2D::create(w, h);

    for (int y = 0; y < h; ++y)
    {
        std::memcpy(&data->at(0, y), image.scanLine(y),
                    static_cast<std::size_t>(w) * sizeof(vsg::vec4));
    }

    auto texture = vsgImGui::Texture::create(data);

    // Добавляем в RenderImGui и в очередь компиляции
    if (context_.render_gui != nullptr)
    {
        context_.render_gui->addChild(texture);

        context_.compile_infos.emplace_back(CompileInfo{
            nullptr, texture, vsg::MASK_ALL});
    }

    return texture;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::ensure_loader()
{
    if (loader_ != nullptr)
    {
        return;
    }

    loader_ = new Loader();
    loader_->owner = this;
    loader_->thread.start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MiniMap::draw()
{
    if (!ImGui::Begin(u8"Карта"))
    {
        ImGui::End();
        return;
    }

    // Позиция камеры (центр карты по умолчанию)
    vsg::dvec3 camera_pos(0.0, 0.0, 0.0);

    if (context_.camera && context_.camera->viewMatrix)
    {
        if (auto look_at = context_.camera->viewMatrix.cast<vsg::LookAt>())
        {
            camera_pos = look_at->eye;
        }
    }

    double cam_lat = route_lat_;
    double cam_lon = route_lon_;
    world_to_latlon(camera_pos.x, camera_pos.y, cam_lat, cam_lon);

    // Управление
    static const char* const styles[] = {u8"OSM", u8"Спутник"};

    ImGui::Combo(u8"Подложка", &map_style_, styles, 2);
    ImGui::SameLine();
    ImGui::Text(u8"zoom %d", zoom_);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
    {
        zoom_ = std::max(3, std::min(19, zoom_ + 1));
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(2))
    {
        zoom_ = std::max(3, std::min(19, zoom_ - 1));
    }

    if (avail.x < 64 || avail.y < 64)
    {
        ImGui::TextDisabled(u8"Слишком мало места");
        ImGui::End();
        return;
    }

    // Пиксель окна -> мировые метры (для клика-телепорта)
    double center_tx = 0.0;
    double center_ty = 0.0;
    latlon_to_tile(cam_lat, cam_lon, center_tx, center_ty);

    const double tile_px = 256.0;

    // Тайловая сетка видимой области
    const double tiles_w = avail.x / tile_px;
    const double tiles_h = avail.y / tile_px;

    const int x0 = static_cast<int>(std::floor(center_tx - tiles_w / 2.0));
    const int x1 = static_cast<int>(std::ceil(center_tx + tiles_w / 2.0));
    const int y0 = static_cast<int>(std::floor(center_ty - tiles_h / 2.0));
    const int y1 = static_cast<int>(std::ceil(center_ty + tiles_h / 2.0));

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(origin,
                       ImVec2(origin.x + avail.x, origin.y + avail.y),
                       true);

    const double n = std::pow(2.0, zoom_);

    auto latlon_to_screen = [&](double lat, double lon) -> ImVec2
    {
        const double tx = (lon + 180.0) / 360.0 * n;
        const double lat_rad = lat * 0.017453292519943295;
        const double ty = (1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / M_PI) / 2.0 * n;

        return ImVec2(origin.x + avail.x / 2.0f + (tx - center_tx) * 256.0f,
                      origin.y + avail.y / 2.0f + (ty - center_ty) * 256.0f);
    };

    auto world_to_screen = [&](double wx, double wy) -> ImVec2
    {
        double lat = route_lat_;
        double lon = route_lon_;
        world_to_latlon(wx, wy, lat, lon);
        return latlon_to_screen(lat, lon);
    };

    // Тайлы
    for (int ty_i = y0; ty_i <= y1; ++ty_i)
    {
        for (int tx_i = x0; tx_i <= x1; ++tx_i)
        {
            TileKey key{zoom_, tx_i, ty_i};
            if (tx_i < 0 || ty_i < 0 || tx_i >= static_cast<int>(n) || ty_i >= static_cast<int>(n))
            {
                continue;
            }

            auto& tile = tiles_[key];

            if (tile.texture == nullptr)
            {
                // Есть готовое изображение -> текстура
                tile.texture = texture_for(key);
            }

            if (tile.texture == nullptr)
            {
                if (!tile.requested)
                {
                    tile.requested = true;
                    request_tile(key);
                }

                // Плейсхолдер
                const ImVec2 p0 = ImVec2(origin.x + avail.x / 2.0f + (tx_i - center_tx) * 256.0f,
                                         origin.y + avail.y / 2.0f + (ty_i - center_ty) * 256.0f);
                draw->AddRectFilled(p0, ImVec2(p0.x + 256.0f, p0.y + 256.0f),
                                    IM_COL32(40, 42, 48, 255));
                continue;
            }

            const ImVec2 p0 = ImVec2(origin.x + avail.x / 2.0f + (tx_i - center_tx) * 256.0f,
                                     origin.y + avail.y / 2.0f + (ty_i - center_ty) * 256.0f);

            draw->AddImage(tile.texture->id(0),
                           p0, ImVec2(p0.x + 256.0f, p0.y + 256.0f));
        }
    }

    // Траектории поверх
    if (context_.topology != nullptr)
    {
        std::lock_guard<std::mutex> lock(context_.topology_mutex);

        const traj_list_t* list = context_.topology->getTrajectoriesList();

        for (auto it = list->cbegin(); it != list->cend(); ++it)
        {
            Trajectory* traj = it.value();
            const bool selected = (traj == context_.selected_trajectory);
            const ImU32 color = selected ? IM_COL32(0, 255, 255, 220)
                                         : IM_COL32(255, 140, 0, 200);

            const double length = traj->getLength();
            const int steps = std::max(2, static_cast<int>(length / 25.0));

            ImVec2 prev = world_to_screen(0.0, 0.0);
            bool first = true;

            for (int i = 0; i <= steps; ++i)
            {
                const double coord = length * i / steps;
                const auto& p = traj->getPosition(coord, 1).position;
                const ImVec2 pt = world_to_screen(p.x, p.y);

                if (!first)
                {
                    draw->AddLine(prev, pt, color, selected ? 2.5f : 1.5f);
                }

                prev = pt;
                first = false;
            }
        }
    }

    // Камера: маркер-стрелка
    {
        const ImVec2 cam = world_to_screen(camera_pos.x, camera_pos.y);
        draw->AddCircleFilled(cam, 5.0f, IM_COL32(80, 160, 255, 255));
        draw->AddCircle(cam, 7.0f, IM_COL32(255, 255, 255, 200));
    }

    // Клик по карте - телепорт камеры
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
    {
        const ImVec2 mp = ImGui::GetIO().MousePos;
        const double px = mp.x - origin.x;
        const double py = mp.y - origin.y;

        // Экран -> мировые метры через Меркатор -> equirectangular
        double tx = center_tx + (px - avail.x / 2.0) / 256.0;
        double ty = center_ty + (py - avail.y / 2.0) / 256.0;

        const double lon = tx / n * 360.0 - 180.0;
        const double lat_rad = std::atan(std::sinh(M_PI * (1.0 - 2.0 * ty / n)));
        const double lat = lat_rad / 0.017453292519943295;

        double wx = 0.0;
        double wy = 0.0;
        latlon_to_world(lat, lon, route_lat_, route_lon_, wx, wy);

        if (context_.camera && context_.camera->viewMatrix)
        {
            if (auto look_at = context_.camera->viewMatrix.cast<vsg::LookAt>())
            {
                const vsg::dvec3 dir = look_at->center - look_at->eye;
                look_at->eye = vsg::dvec3(wx, wy, look_at->eye.z);
                look_at->center = look_at->eye + dir;
            }
        }
    }

    draw->PopClipRect();

    ImGui::End();
}
