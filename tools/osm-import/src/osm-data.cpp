//------------------------------------------------------------------------------
//
//      osm-import: парсинг OSM XML, Overpass-загрузка, склейка путей
//
//------------------------------------------------------------------------------

#include "osm-data.h"

#include <QDomDocument>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace osm
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool parse_osm_xml(const std::string& content, OsmData& out, std::string* error)
{
    QDomDocument doc;
    QString parse_error = "";
    int line = 0;
    int column = 0;

    if (!doc.setContent(QByteArray(content.c_str(), static_cast<int>(content.size())),
                        &parse_error, &line, &column))
    {
        if (error != nullptr)
        {
            *error = "XML parse error at " + std::to_string(line) + ":" +
                     std::to_string(column) + ": " + parse_error.toStdString();
        }
        return false;
    }

    const QDomElement root = doc.documentElement();

    // Overpass возвращает <osm> напрямую; при response-formatter=json
    // сюда попадать не должны
    if (root.isNull() || root.tagName() != "osm")
    {
        if (error != nullptr)
        {
            *error = "Root element is not <osm>";
        }
        return false;
    }

    double min_lat = 1e9;
    double max_lat = -1e9;
    double min_lon = 1e9;
    double max_lon = -1e9;

    QDomElement el = root.firstChildElement();
    while (!el.isNull())
    {
        if (el.tagName() == "node")
        {
            Node node;
            node.id = el.attribute("id").toULongLong();
            node.lat = el.attribute("lat").toDouble();
            node.lon = el.attribute("lon").toDouble();
            out.nodes[node.id] = node;

            min_lat = std::min(min_lat, node.lat);
            max_lat = std::max(max_lat, node.lat);
            min_lon = std::min(min_lon, node.lon);
            max_lon = std::max(max_lon, node.lon);
        }
        else if (el.tagName() == "way")
        {
            Way way;
            way.id = el.attribute("id").toULongLong();

            QDomElement child = el.firstChildElement();
            while (!child.isNull())
            {
                if (child.tagName() == "nd")
                {
                    const std::uint64_t ref = child.attribute("ref").toULongLong();
                    way.nodes.push_back(ref);

                    // out geom: координаты прямо в nd - строим ноды
                    // без отдельного прохода по <node>
                    if (child.hasAttribute("lat") && child.hasAttribute("lon"))
                    {
                        Node node;
                        node.id = ref;
                        node.lat = child.attribute("lat").toDouble();
                        node.lon = child.attribute("lon").toDouble();
                        out.nodes[ref] = node;
                    }
                }
                else if (child.tagName() == "tag")
                {
                    const QString key = child.attribute("k");
                    const QString value = child.attribute("v");

                    if (key == "railway") way.railway = value.toStdString();
                    else if (key == "usage") way.usage = value.toStdString();
                    else if (key == "service") way.service = value.toStdString();
                    else if (key == "name") way.name = value.toStdString();
                    else if (key == "gauge") way.gauge = value.toStdString();
                    else if (key == "electrified") way.electrified = value.toStdString();
                }

                child = child.nextSiblingElement();
            }

            if (!way.nodes.empty())
            {
                out.ways.push_back(std::move(way));
            }
        }

        el = el.nextSiblingElement();
    }

    // BBox из <bounds>, иначе - по нодам
    const QDomElement bounds = root.firstChildElement("bounds");
    if (!bounds.isNull())
    {
        min_lat = bounds.attribute("minlat").toDouble();
        min_lon = bounds.attribute("minlon").toDouble();
        max_lat = bounds.attribute("maxlat").toDouble();
        max_lon = bounds.attribute("maxlon").toDouble();
    }

    out.min_lat = min_lat;
    out.max_lat = max_lat;
    out.min_lon = min_lon;
    out.max_lon = max_lon;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool parse_osm_file(const std::string& path, OsmData& out, std::string* error)
{
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = "Can't open file: " + path;
        }
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    return parse_osm_xml(std::string(data.constData(), static_cast<std::size_t>(data.size())),
                         out, error);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool download_overpass(const std::string& overpass_url,
                       double min_lat, double min_lon,
                       double max_lat, double max_lon,
                       std::string& xml_out, std::string* error)
{
    // Все железные дороги bbox: главные + станционные/подъездные
    // (service-пути отфильтруются по флагу)
    // Лёгкий запрос: out geom отдаёт координаты точек прямо внутри
    // <way>/<nd>, без дорогой рекурсии (._;>;) по всем нодам
    const QString query = QString(
        "[out:xml][timeout:180];"
        "("
        "way[\"railway\"=\"rail\"](%1,%2,%3,%4);"
        "way[\"railway\"=\"light_rail\"](%1,%2,%3,%4);"
        "way[\"railway\"=\"narrow_gauge\"](%1,%2,%3,%4);"
        ");"
        "out geom;")
        .arg(min_lat, 0, 'f', 7)
        .arg(min_lon, 0, 'f', 7)
        .arg(max_lat, 0, 'f', 7)
        .arg(max_lon, 0, 'f', 7);

    const QUrl url(QString::fromStdString(overpass_url));

    QNetworkAccessManager manager;
    QEventLoop loop;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "RRS-osm-import/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setTransferTimeout(300000);

    // POST как у curl: длинные GET-URL Overpass держит неохотно
    const QByteArray body = "data=" + QUrl::toPercentEncoding(query).toPercentEncoding();

    QNetworkReply* reply = manager.post(request, body);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        if (error != nullptr)
        {
            *error = "Overpass error: " + reply->errorString().toStdString();
        }
        reply->deleteLater();
        return false;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    xml_out.assign(data.constData(), static_cast<std::size_t>(data.size()));

    if (xml_out.size() < 100)
    {
        if (error != nullptr)
        {
            *error = "Overpass returned empty response";
        }
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<Way> filter_railways(const OsmData& data, bool include_service)
{
    std::vector<Way> result;

    for (const Way& way : data.ways)
    {
        // Только железнодорожные пути (трамваи/метро по умолчанию нет:
        // их нет в запросе; sidings/yard - по флагу)
        if (way.railway != "rail" &&
            way.railway != "light_rail" &&
            way.railway != "narrow_gauge")
        {
            continue;
        }

        // Заброшенные/снятые пути пропускаем всегда
        // (railway=disused/bandoned/construction приходят другими тегами)

        if (!include_service && !way.service.empty())
        {
            continue;
        }

        result.push_back(way);
    }

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<Chain> build_chains(const OsmData& data,
                                const std::vector<Way>& ways)
{
    // Степени нод: сколько раз нода встречается в way (концы way считаются)
    std::map<std::uint64_t, int> degree;
    std::set<std::uint64_t> used_nodes;

    for (const Way& way : ways)
    {
        if (way.nodes.empty())
        {
            continue;
        }

        for (std::size_t i = 0; i < way.nodes.size(); ++i)
        {
            const std::uint64_t id = way.nodes[i];

            // Промежуточные ноды не создают разветвлений, если way
            // проходит их насквозь; но в OSM way делится на сегменты
            // только на общих нодах, поэтому считаем вхождения честно:
            // внутренняя нода одного way = степень 1 (сквозная),
            // концы могут совпадать с другими way
            if (i == 0 || i == way.nodes.size() - 1)
            {
                degree[id] += 1;
            }
        }
    }

    // Рёбра: последовательные пары нод way
    struct Edge
    {
        std::uint64_t a = 0;
        std::uint64_t b = 0;
        const Way* way = nullptr;
        bool used = false;
    };

    std::vector<Edge> edges;

    for (const Way& way : ways)
    {
        for (std::size_t i = 0; i + 1 < way.nodes.size(); ++i)
        {
            Edge edge;
            edge.a = way.nodes[i];
            edge.b = way.nodes[i + 1];
            edge.way = &way;

            // Пропускаем петли-мусор
            if (edge.a == edge.b)
            {
                continue;
            }

            edges.push_back(edge);
        }
    }

    // Индекс: нода -> индексы рёбер
    std::map<std::uint64_t, std::vector<std::size_t>> by_node;

    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        by_node[edges[i].a].push_back(i);
        by_node[edges[i].b].push_back(i);
    }

    // Склейка: начинаем с рёбер, у которых конец - "особая" нода
    // (degree != 1), либо с любого неиспользованного
    auto extend = [&](Chain& chain, std::uint64_t from, std::size_t edge_idx)
    {
        Edge& edge = edges[edge_idx];
        edge.used = true;

        std::uint64_t current = (edge.a == from) ? edge.b : edge.a;
        const Way* way = edge.way;

        chain.node_ids.push_back(current);

        // Продолжаем, пока текущая нода "сквозная" для способа
        // (принадлежит ровно одному way-набору и degree == 1)
        while (true)
        {
            const auto node_edges = by_node.find(current);

            // Ищем неиспользованное ребро с тем же way
            std::size_t next_idx = edges.size();
            int same_way_unused = 0;

            if (node_edges != by_node.end())
            {
                for (std::size_t idx : node_edges->second)
                {
                    if (edges[idx].used)
                    {
                        continue;
                    }

                    // Приоритет: тот же way (продолжение прямое)
                    if (edges[idx].way == way)
                    {
                        next_idx = idx;
                        ++same_way_unused;
                    }
                }

                // Нет продолжения тем же way: если нода сквозная для
                // других путей с теми же тегами - продолжаем по ним
                if (next_idx == edges.size())
                {
                    for (std::size_t idx : node_edges->second)
                    {
                        if (!edges[idx].used)
                        {
                            if (same_way_unused == 0)
                            {
                                next_idx = idx;
                            }
                            break;
                        }
                    }
                }
            }

            if (next_idx == edges.size())
            {
                break;
            }

            Edge& next = edges[next_idx];
            next.used = true;

            current = (next.a == current) ? next.b : next.a;
            way = next.way;
            chain.node_ids.push_back(current);
        }
    };

    std::vector<Chain> chains;

    // 1) Рёбра, начинающиеся в особых точках
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (edges[i].used)
        {
            continue;
        }

        const bool a_junction = (degree.count(edges[i].a) == 0) ||
                                (degree.at(edges[i].a) != 1);
        const bool b_junction = (degree.count(edges[i].b) == 0) ||
                                (degree.at(edges[i].b) != 1);

        // Начинаем только если один конец - развилка/тупик
        if (!a_junction && !b_junction)
        {
            continue;
        }

        Chain chain;

        const std::uint64_t start = a_junction ? edges[i].a : edges[i].b;
        chain.node_ids.push_back(start);
        extend(chain, start, i);

        chains.push_back(std::move(chain));
    }

    // 2) Остальные (замкнутые контуры без особых точек)
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (edges[i].used)
        {
            continue;
        }

        Chain chain;
        chain.node_ids.push_back(edges[i].a);
        extend(chain, edges[i].a, i);
        chains.push_back(std::move(chain));
    }

    // Метаданные и чистка
    std::map<const Way*, int> way_counter;

    for (Chain& chain : chains)
    {
        if (chain.node_ids.size() < 2)
        {
            continue;
        }

        chain.railway = "rail";
    }

    // Убираем вырожденные
    chains.erase(std::remove_if(chains.begin(), chains.end(),
                                [](const Chain& c)
                                {
                                    return c.node_ids.size() < 2;
                                }),
                 chains.end());

    return chains;
}

} // namespace osm
